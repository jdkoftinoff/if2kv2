#ifndef __IF2_SCANNER_H
# define __IF2_SCANNER_H

# include "jdk_tree.h"
# include "jdk_dynbuf.h"
# include "jdk_remote_buf.h"
# include "jdk_lz.h"
# include "jdk_thread.h"
# include "if2k_pattern_expander.h"

template <class VALUE_T>
struct if2_tree_nonalphanumericpunct_ignorer
{
    inline bool operator() ( const VALUE_T &a ) const
    {
        return !jdk_isalnum(a) && !jdk_ispunct(a);
	}
};



struct if2_tree_traits_alphanumeric
{	
	typedef char value_t;
	typedef int index_t;
	typedef short flags_t;
	typedef jdk_dynbuf buf_t;
	typedef jdk_tree_case_insensitive_comparator<value_t> comparator_t;
	typedef if2_tree_nonalphanumericpunct_ignorer<value_t> ignorer_t;
	typedef jdk_tree_basic_leaf< value_t, index_t, flags_t > leaf_t;
	enum { required_prefix_flag = false };
	static inline bool test_prefix( value_t v ) { return false; }
};

struct if2_tree_traits_raw
{	
	typedef char value_t;
	typedef int index_t;
	typedef short flags_t;
	typedef jdk_dynbuf buf_t;
	typedef jdk_tree_case_insensitive_comparator<value_t> comparator_t;
	typedef jdk_tree_basic_ignorer<value_t> ignorer_t;
	typedef jdk_tree_basic_leaf< value_t, index_t, flags_t > leaf_t;
	enum { required_prefix_flag = false };
	static inline bool test_prefix( value_t v ) { return false; }
};

struct if2_tree_traits_url
{
	typedef char value_t;
	typedef int index_t;
	typedef short flags_t;
	typedef jdk_dynbuf buf_t;
	typedef jdk_tree_case_insensitive_comparator<value_t> comparator_t;
	typedef jdk_tree_basic_ignorer<value_t> ignorer_t;
	typedef jdk_tree_basic_leaf< value_t, index_t, flags_t > leaf_t;
	enum { required_prefix_flag = true };
	static inline bool test_prefix( value_t v ) { return ( v=='.' || v=='/' || v=='=' || v=='&'); }
};


template <class tree_traits>
class if2_scanner_event
{
	public:
    typedef tree_traits tree_traits_t;	
	if2_scanner_event() 
	{
	}
	
	virtual ~if2_scanner_event()
	{
	}
	
	virtual void operator () ( const jdk_tree<tree_traits> &tree, typename tree_traits::index_t match_item ) = 0;
};




template <class tree_traits>
class if2_scanner_tree : public if2_pattern_target
{
//	bool oops;
	
	public:
    typedef tree_traits tree_traits_t;	
	
	explicit if2_scanner_tree()
		: /*oops(false), */ tree( 1000 ), mutex("scanner_tree")
	{
		for( int i=0; i<256; ++i )
		{
			initial_shortcuts[i]=-1;	
		}		
	}
	
	virtual ~if2_scanner_tree()
	{
		
	}
	
	
	void clear()
	{
		jdk_synchronized(mutex);		
		tree.clear();	
		for( int i=0; i<256; ++i )
		{
			initial_shortcuts[i]=-1;	
		}	  
	}
	
	bool load( const jdk_dynbuf &buf )
	{
		jdk_synchronized(mutex);		
		clear();
		jdk_lz_decompress( &buf, &tree.getbuf() );
		make_shortcuts();
		return true;
	}	
	
	bool save( jdk_dynbuf &buf ) const
	{
		jdk_synchronized(mutex);		
		jdk_lz_compress( &tree.getbuf(), &buf );
		return true;		
	}
	
	
	void add( const jdk_string &s, typename tree_traits::flags_t flags )
	{
		jdk_synchronized(mutex);		
		//jdk_log( JDK_LOG_INFO, "%d %s",  (int)flags, s.c_str() );
#if 0		
		if( s.len()<14 )
		{
			fprintf( stderr, "found it:\n" );
			oops=true;
		}
		if( oops )
		{
			fprintf( stderr, "%s\n", s.c_str() );			
		}
#endif		
		
		tree.add( s.c_str(), s.len(), flags );				
	}
	
	void remove( const jdk_string &s )
	{
		jdk_synchronized(mutex);		
	    // TODO: remove  tree.remove( s.c_str(), s.len() );
	}
	
	
	bool find( 
			  const void *buf, 
			  int buf_len,
			  if2_scanner_event<tree_traits> &event
			  ) const
	{
		jdk_synchronized(mutex);		
		int match_count=0;
		typename tree_traits::flags_t flags;
		typename tree_traits::index_t match_item;
		
		const typename tree_traits::value_t *p = (const typename tree_traits::value_t *)buf;

		if( tree_traits::required_prefix_flag )
		{
			for( int i=0; i<buf_len; ++i, ++p )
			{
				if( tree_traits::test_prefix( *p ) )
				{
					int first_value = p[1];
					if( first_value<0 )
						first_value+=128;
					
					typename tree_traits::index_t shortcut =initial_shortcuts[first_value];
					if( shortcut != -1 )
					{
						if( tree.find_longest(p+1,buf_len-i-1,&flags,0,&match_item, shortcut, 0 ) )
						{
							// found a match!
							++match_count;
							event(tree,match_item);
						}
					}
				}
			}
		}
		else
		{
			for( int i=0; i<buf_len; ++i, ++p )
			{		
				int first_value = *p;
				if( first_value<0 )
				first_value+=128;
				
				typename tree_traits::index_t shortcut =initial_shortcuts[first_value];
				if( shortcut != -1 )
				{
					if( tree.find_longest( p, buf_len-i, &flags, 0, &match_item, shortcut, 0 ) )
					{
						// found a match!
						++match_count;
						event( tree, match_item );
					}			
				}
			}
		}
		
		return match_count!=0;
	}
	
	
	
	bool find( 
			  const jdk_dynbuf &buf, 
			  if2_scanner_event<tree_traits> &event
			  ) const
	{
		jdk_synchronized(mutex);		
		return find( buf.get_data(), buf.get_data_length(), event );
	}
	
	void make_shortcuts()
	{
		jdk_synchronized(mutex);
		// start at first entry, go through all siblings and extract item offsets for each entry
		for( int i=0; i<256; ++i )
		{
			initial_shortcuts[i]=-1;	
		}
		
		typename tree_traits::index_t item=0;
		
		do
		{
			const typename tree_traits::leaf_t &leaf = tree.get( item );
			if( leaf.is_free() || leaf.is_end() )
			  break;
			if( jdk_isalpha( leaf.value ) )
			{
				initial_shortcuts[ (int)jdk_toupper(leaf.value) ] = item;			
				initial_shortcuts[ (int)jdk_tolower(leaf.value) ] = item;						
			}
			else
			{
				initial_shortcuts[ (int)leaf.value ] = item;
			}
			item = leaf.sibling;
		} while( item>0 );		
	}
	
    int extract( 
				typename tree_traits::value_t *list, 
				int max_len,
				typename tree_traits::index_t end_leaf_index 
				) const
	{
		jdk_synchronized(mutex);
		return tree.extract( list, max_len, end_leaf_index );
	}
	
private:
	jdk_tree<tree_traits> tree;
	typename tree_traits::index_t initial_shortcuts[256];
	mutable jdk_recursivemutex mutex;
};

template <class tree_traits>
class if2_scanner
{
	public:
	
	typedef tree_traits tree_traits_t;
	explicit if2_scanner( 
						 const jdk_settings &settings, 
						 const jdk_string &precompiled_setting,
						 const jdk_string &setting_prefix,
						 if2_pattern_expander &expander,
						 int num_files,
						 int code_offset,
						 const jdk_string &pattern_prefix_
						 )
		: pattern_prefix( pattern_prefix_ ), tree(), mutex( "if2_scanner" )
	{
		// load precompiled settings
		{
			jdk_remote_buf buf(settings,precompiled_setting,10*1024*1024,false);
			buf.check_and_grab();
			if( buf.get_buf().get_data_length()>0 )
			{
        	  jdk_synchronized( mutex );			
			  tree.load( buf.get_buf() );
			}
		}
		
		// load remote buffers and local files
		for( int i=0; i<num_files; ++i )
		{
			jdk_str<4096> real_prefix;
			real_prefix.cpy( setting_prefix );
			
			if( i!=0 )
			{
				jdk_str<128> num;
				num.form("%d", i );
				real_prefix.cat( num );
			}

			jdk_log( JDK_LOG_DEBUG4, "real_prefix: %s", real_prefix.c_str() );


			// check for builtin categories
			if( settings.get_prefixed_long(real_prefix,"_default_enable") )
			{
				jdk_str<1024> fname = settings.get_prefixed(real_prefix,"_default");
				if( !fname.is_clear() )
				{
					jdk_dynbuf buf;
					jdk_log( JDK_LOG_DEBUG4, "About to load embedded: %s %s", real_prefix.c_str(), fname.c_str() );
					jdk_lz_decompress( "default", fname.c_str(), &buf );

					jdk_synchronized( mutex );
					expander.load( buf, tree, code_offset+i, pattern_prefix );
					jdk_log( JDK_LOG_DEBUG4, "bufsize is %d", buf.get_data_length() );
				}
			}

			// check for remote categories
			if( settings.get_prefixed_long(real_prefix,"_url_enable") )
			{
				jdk_log( JDK_LOG_DEBUG4, "About to load url: %s", real_prefix.c_str() );
				jdk_remote_buf buf(settings,real_prefix,8*1024*1024,false);
				buf.check_and_grab();
				jdk_log( JDK_LOG_DEBUG4, "bufsize is %d", buf.get_buf().get_data_length() );
				jdk_synchronized( mutex );
				// load the remote buffer
				expander.load(buf.get_buf(),tree,code_offset+10+i,pattern_prefix);
			}
			
			// check for local file categories
			if( settings.get_prefixed_long(real_prefix,"_file_enable") )
			{
				jdk_log( JDK_LOG_DEBUG4, "About to load file: %s", real_prefix.c_str() );
			
				expander.load(
				  settings.get_prefixed( real_prefix, "_file" ),
				  tree,
				  code_offset+20+i,
				  pattern_prefix
				  );
			}
		}

		tree.make_shortcuts();
	}
	
	virtual ~if2_scanner() {}
	
	jdk_mutex &get_mutex() const
	{
	    return mutex;
	}
	
	bool add_pattern(const jdk_string &p,if2_pattern_expander &expander, short flags )
	{
	    jdk_synchronized( mutex );
        return expander.expand( p, expander, flags, false, pattern_prefix );
	}

	bool remove_pattern(const jdk_string &p,if2_pattern_expander &expander )
	{
	    jdk_synchronized( mutex );
        return expander.expand( p, expander, 0, true, pattern_prefix );
	}

	bool add_patterns(const jdk_dynbuf &buf,if2_pattern_expander &expander, short flags )
	{
	    jdk_synchronized( mutex );
        return expander.load( buf, expander, flags, false, pattern_prefix );
	}

	bool remove_pattern(const jdk_dynbuf &buf,if2_pattern_expander &expander )
	{
	    jdk_synchronized( mutex );
        return expander.load_remove( p, expander, 0,  pattern_prefix );
	}
	
	bool add_patterns(const jdk_string &fname,if2_pattern_expander &expander, short flags )
	{
	    jdk_synchronized( mutex );
        return expander.load( fname, expander, flags,  pattern_prefix );
	}

	bool remove_patterns(const jdk_string &buf,if2_pattern_expander &expander )
	{
	    jdk_synchronized( mutex );
        return expander.load_remove( fname, expander, 0,  pattern_prefix );
	}
	
		
		
	bool find( 
			  const void *buf, 
			  int buf_len,
			  if2_scanner_event<tree_traits> &event
			  ) const
	{
	    jdk_synchronized( mutex );
		return tree.find( buf,buf_len,event );
	}
	
	bool find( 
			  const jdk_dynbuf &buf, 
			  if2_scanner_event<tree_traits> &event
			  ) const
	{
	    jdk_synchronized( mutex );
		return tree.find( buf, event );
	}
	
    int extract( 
				typename tree_traits::value_t *list, 
				int max_len, 
				typename tree_traits::index_t end_leaf_index 
				) const
	{
	    jdk_synchronized( mutex );
		return tree.extract( list, max_len, end_leaf_index );
	}
	
	
	if2_scanner_tree<tree_traits> &get_tree() { return tree; }
	const if2_scanner_tree<tree_traits> &get_tree() const { return tree; }
	
private:
	jdk_str<256> pattern_prefix;
	if2_scanner_tree<tree_traits> tree;
    mutable jdk_recursivemutex mutex;
};

typedef if2_scanner<if2_tree_traits_alphanumeric> if2_scanner_alphanumeric;
typedef if2_scanner<if2_tree_traits_raw> if2_scanner_raw;


#endif
