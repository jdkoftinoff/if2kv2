#ifndef __IF2K_MINI_TREE_H
#define __IF2K_MINI_TREE_H

#include "jdk_dynbuf.h"
#include "jdk_tree.h"
#include "jdk_mmap_buf.h"
#include "if2k_pattern_expander.h"
#include "jdk_log.h"

template <class VALUE_T>
struct if2k_mini_tree_nonalphanumericpunct_ignorer
{
  inline bool operator() ( const VALUE_T &a ) const
  {
    return !jdk_isalnum(a) && !jdk_ispunct(a);
  }
};



struct if2k_mini_tree_traits_alphanumeric
{	
	typedef char value_t;
	typedef int index_t;
	typedef short flags_t;
	typedef jdk_dynbuf buf_t;
	typedef jdk_tree_case_insensitive_comparator<value_t> comparator_t;
	typedef if2k_mini_tree_nonalphanumericpunct_ignorer<value_t> ignorer_t;
	typedef jdk_tree_basic_leaf< value_t, index_t, flags_t > leaf_t;
	enum { required_prefix_flag = false };
	static inline bool test_prefix( value_t v ) { return false; }
};


struct if2k_mini_tree_traits_url
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

struct if2k_mini_tree_traits_alphanumeric_mmap
{	
	typedef char value_t;
	typedef int index_t;
	typedef short flags_t;
	typedef jdk_mmap_buf buf_t;
	typedef jdk_tree_case_insensitive_comparator<value_t> comparator_t;
	typedef if2k_mini_tree_nonalphanumericpunct_ignorer<value_t> ignorer_t;
	typedef jdk_tree_basic_leaf< value_t, index_t, flags_t > leaf_t;
	enum { required_prefix_flag = false };
	static inline bool test_prefix( value_t v ) { return false; }
};


struct if2k_mini_tree_traits_url_mmap
{
	typedef char value_t;
	typedef int index_t;
	typedef short flags_t;
	typedef jdk_mmap_buf buf_t;
	typedef jdk_tree_case_insensitive_comparator<value_t> comparator_t;
	typedef jdk_tree_basic_ignorer<value_t> ignorer_t;
	typedef jdk_tree_basic_leaf< value_t, index_t, flags_t > leaf_t;
	enum { required_prefix_flag = true };
	static inline bool test_prefix( value_t v ) { return ( v=='.' || v=='/' || v=='=' || v=='&'); }
};


template <class tree_traits>
class if2k_mini_tree_event
{
public:
  typedef tree_traits tree_traits_t;	
	if2k_mini_tree_event() 
    {
    }
	
	virtual ~if2k_mini_tree_event()
    {
    }
	
	virtual void operator () ( const jdk_tree<tree_traits> &tree, typename tree_traits::index_t match_item ) {}
};




template <class tree_traits>
class if2k_mini_tree : public if2_pattern_target
{	
public:
  typedef tree_traits tree_traits_t;	

	explicit if2k_mini_tree()
		: 
    tree( 1000 ),
    tree_type( "compile" ),
    tree_category( "" )
    {
      for( size_t i=0; i<256; ++i )
      {
        initial_shortcuts[i]=-1;	
      }		
    }

	explicit if2k_mini_tree( const char *tree_type_, const jdk_string &tree_category_)
		: 
    tree( 1000 ),
    tree_type( tree_type_ ),
    tree_category( tree_category_ )
    {
      for( size_t i=0; i<256; ++i )
      {
        initial_shortcuts[i]=-1;	
      }		
    }
	
	explicit if2k_mini_tree( const jdk_string &tree_type_, const jdk_string &tree_category_)
		: 
    tree( 1000 ),
    tree_type( tree_type_ ),
    tree_category( tree_category_ )
    {
      for( size_t i=0; i<256; ++i )
      {
        initial_shortcuts[i]=-1;	
      }		
    }

	explicit if2k_mini_tree( const jdk_string_filename &filename )
		: 
    tree( filename ),
    tree_type( "" ),
    tree_category( "" )
    {
      make_shortcuts();
    }

	explicit if2k_mini_tree( const jdk_string_filename &filename, const char *tree_type_, const jdk_string &tree_category_ )
		: 
    tree( filename ),
    tree_type( tree_type_ ),
    tree_category( tree_category_ )
    {
      make_shortcuts();
    }

	explicit if2k_mini_tree( const jdk_string_filename &filename, const jdk_string &tree_type_, const jdk_string &tree_category_ )
		: 
    tree( filename ),
    tree_type( tree_type_ ),
    tree_category( tree_category_ )
    {
      make_shortcuts();
    }
	
	virtual ~if2k_mini_tree()
    {
      
    }
	
	
	void clear()
    {
      tree.clear();	
      for( size_t i=0; i<256; ++i )
      {
        initial_shortcuts[i]=-1;	
      }	  
    }
	
	bool load_compressed( const jdk_buf &buf )
    {
      clear();
      jdk_lz_decompress( &buf, &tree.getbuf() );
      make_shortcuts();
      return true;
    }	
	
	bool save_compressed( jdk_dynbuf &buf ) const
    {
      jdk_lz_compress( &tree.getbuf(), &buf );
      return true;		
    }

	bool load_uncompressed( const jdk_buf &buf )
    {
      bool r = tree.getbuf().copy(buf);
      make_shortcuts();
      return r;
    }	
	
	bool save_uncompressed( jdk_dynbuf &buf ) const
    {
      return buf.copy(tree.getbuf());
    }
	
	
	void add( const jdk_string &s, typename tree_traits::flags_t flags )
    {
      tree.add( s.c_str(), s.len(), flags );				
    }
	
	void remove( const jdk_string &s )
    {
    }
	
	
	bool find( 
    const void *buf, 
    size_t buf_len,
    if2k_mini_tree_event<tree_traits> &event
    ) const
    {
      int match_count=0;
      typename tree_traits::flags_t flags;
      typename tree_traits::index_t match_item;
      
      const typename tree_traits::value_t *p = (const typename tree_traits::value_t *)buf;
      
      if( buf_len<4 || buf==0 ) return false;
      
      if( tree_traits::required_prefix_flag )
      {
        for( size_t i=0; i<buf_len-1; ++i, ++p )
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
                jdk_log( JDK_LOG_DEBUG3, "FOUND MATCH (1)" );
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
        for( size_t i=0; i<buf_len; ++i, ++p )
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
              jdk_log( JDK_LOG_DEBUG1, "FOUND MATCH (2)" );
              ++match_count;
              event( tree, match_item );
            }			
          }
        }
      }
      
      return match_count!=0;
    }
	
	
	
	bool find( 
    const jdk_buf &buf, 
    if2k_mini_tree_event<tree_traits> &event
    ) const
    {
      return find( buf.get_data(), buf.get_data_length(), event );
    }

	bool censor( 
    void *buf, 
    size_t buf_len,
    if2k_mini_tree_event<tree_traits> &event,
    typename tree_traits::value_t replacement
    ) const
    {
      int match_count=0;
      typename tree_traits::flags_t flags;
      typename tree_traits::index_t match_item;
      
      typename tree_traits::value_t *p = (typename tree_traits::value_t *)buf;
      
      if( tree_traits::required_prefix_flag )
      {
        for( size_t i=0; i<buf_len-1; ++i, ++p )
        {
          if( tree_traits::test_prefix( *p ) )
          {
            int first_value = p[1];
            if( first_value<0 )
              first_value+=128;
            
            typename tree_traits::index_t shortcut = initial_shortcuts[first_value];
            if( shortcut != -1 )
            {
              if( tree.censor_longest(p+1,buf_len-i-1,replacement,&flags,0,&match_item, shortcut, 0 ) )
              {
                jdk_log( JDK_LOG_DEBUG3, "CENSORED MATCH (1)" );
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
        for( size_t i=0; i<buf_len; ++i, ++p )
        {		
          int first_value = *p;
          if( first_value<0 )
            first_value+=128;
          
          typename tree_traits::index_t shortcut =initial_shortcuts[first_value];
          if( shortcut != -1 )
          {
            if( tree.censor_longest( p, buf_len-i, replacement, &flags, 0, &match_item, shortcut, 0 ) )
            {
              // found a match!
              jdk_log( JDK_LOG_DEBUG1, "CENSORED MATCH (2)" );
              ++match_count;
              event( tree, match_item );
            }			
          }
        }
      }
      
      return match_count!=0;
    }
	
	bool censor( 
    jdk_buf &buf, 
    if2k_mini_tree_event<tree_traits> &event,
    typename tree_traits::value_t replacement
    ) const
    {
      return censor( buf.get_data(), buf.get_data_length(), event, replacement );
    }

	
	void make_shortcuts()
    {
      
      // start at first entry, go through all siblings and extract item offsets for each entry
      for( size_t i=0; i<256; ++i )
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
	
  size_t extract( 
    typename tree_traits::value_t *list, 
    size_t max_len,
    typename tree_traits::index_t end_leaf_index 
    ) const
    {
      return tree.extract( list, max_len, end_leaf_index );
    }
  
  size_t extract( 
    jdk_string &result, 				
    typename tree_traits::index_t end_leaf_index
    ) const
    {
      size_t maxlen=result.getmaxlen();
      for( size_t i=0; i<maxlen; ++i )
      {
        result.set(i,0);
      }

      return tree.extract(
        (typename tree_traits::value_t *)result.getaddr(0), 
        result.getmaxlen()-1,
        end_leaf_index
        );
    }

  const jdk_string &get_tree_type() const
    {
      return tree_type;
    }

  const jdk_string &get_tree_category() const
    {
      return tree_category;
    }
	
private:
	jdk_tree<tree_traits> tree;
	typename tree_traits::index_t initial_shortcuts[256];
  jdk_str<128> tree_type;
  jdk_str<128> tree_category;
};

template <class tree_traits,class pattern_expander>
inline bool if2k_mini_tree_compile( 
  const jdk_string_filename &src,
  const jdk_string_filename &dest,
  int code_offset,
  const jdk_string &prefix,
  volatile int *progress=0
  )
{
  bool r=false;
  if2k_mini_tree<tree_traits> tree;
  pattern_expander expander;

  jdk_dynbuf input_buf;
  if(
    input_buf.append_from_file( src )
    &&  expander.load( input_buf, tree, code_offset, prefix, progress )
    )
  {  
    tree.make_shortcuts();

    jdk_dynbuf results;
    if( tree.save_uncompressed( results ) )
    {
      if( results.extract_to_file( dest ) )
      {
        r = true;
      }
    }
  }
  return r;
}


#endif
