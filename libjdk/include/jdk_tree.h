#ifndef _JDK_TREE_H
#define _JDK_TREE_H

#include "jdk_string.h"
#include "jdk_error.h"

// this is tricky stuff.
// the class is made so that the guts of the data structure can be memory mapped,
// loaded raw from a file, put in shared memory, or whatever. The leaf type must be
// a trivial type, containing no pointers or virtual methods!
//
//
// requirements of TRAITS_T: (example)
//
//    struct example_tree_traits
//    {
//        typedef char value_t;
//        typedef int index_t;
//        typedef char flags_t;
//        typedef jdk_dynbuf buf_t;
//        typedef jdk_tree_basic_comparator<value_t> comparator_t;
//        typedef jdk_tree_basic_ignorer<value_t> ignorer_t;
//        typedef jdk_tree_basic_leaf< value_t, index_t, flags_t > leaf_t;
//
//    };
//
//    a custom leaf type looks like:
//
//        struct leaf_t
//        {
//          leaf_t() : T(), sibling(), child(), flags()
//	        {
//    	    }
//	
//    	    bool is_free() const { return value==0; } // or something like this
//	        void free() { value=0; } // or something like this
//          void allocate() {}    // or set a separate flag
//
//	    
//	        index_t sibling;
//	        index_t child;
//          index_t parent;
//          value_t value;
//	        flags_t flags;
//        };
//
//


// requirements of BUF_T:
//    must have constructor which takes a filename
//    must have constructor which takes a size in bytes
//    must have copy constructor
//    must have the methods:
//        void *get_data() // or void *
//        int get_data_length() // in bytes
//        bool expand( int new_size_in_bytes );
//        void fill(); // make data_length same as buffer length
//
//    of course a jdk_dynbuf will work
//    as well a jdk_sharedbuf


template <class VALUE_T>
struct jdk_tree_basic_ignorer
{
   inline bool operator() ( const VALUE_T &a ) const
   {
      return false;
   }
};



template <class VALUE_T>
struct jdk_tree_whitespace_ignorer
{
   inline bool operator() ( const VALUE_T &a ) const
   {
      return jdk_isspace(a);
   }
};

template <class VALUE_T>
struct jdk_tree_nonalphanumeric_ignorer
{
   inline bool operator() ( const VALUE_T &a ) const
   {
      return !jdk_isalnum(a);
   }
};

template <class VALUE_T>
struct jdk_tree_basic_comparator
{
   inline int operator() ( const VALUE_T &a, const VALUE_T &b ) const
   {
      int result;

      if( a==b )
         result=0;
      else if( a<b )
         result=-1;
      else
         result=1;	    
      return result;
   }
};


template <class VALUE_T>
struct jdk_tree_case_insensitive_comparator
{
   inline int operator() ( const VALUE_T &a_, const VALUE_T &b_ ) const
   {
      int result;
      VALUE_T a( jdk_toupper(a_) );
      VALUE_T b( jdk_toupper(b_) );

      if( a==b )
         result=0;
      else if( a<b )
         result=-1;
      else
         result=1;	    
      return result;
   }
};



template <class VALUE_T, class INDEX_T, class FLAGS_T>
struct jdk_tree_basic_leaf
{
   inline jdk_tree_basic_leaf()
      : sibling(0), child(0), parent(0), flags(0), value(0), in_use(0)
   {
   }

   inline void clear()
   {
      sibling=0;
      child=0;
      parent=0;
      flags=0;
      value=0;
      in_use=0;
   }

   inline bool is_free() const { return (in_use&1)==0; }
   inline void release() { in_use=0; }
   inline void allocate() { in_use|=1; }

   inline bool is_end() const { return bool((in_use&2) !=0); }	
   inline void set_end() { in_use|=2; }
   inline void unset_end() { in_use=in_use&1; }

   INDEX_T sibling;
   INDEX_T child;
   INDEX_T parent;
   FLAGS_T flags;
   VALUE_T value;	
   unsigned char in_use;    
};


template <typename TRAITS_T>
class jdk_tree
{
public:
   typedef typename TRAITS_T::value_t value_t;
   typedef typename TRAITS_T::index_t index_t;
   typedef typename TRAITS_T::flags_t flags_t;
   typedef typename TRAITS_T::buf_t buf_t;
   typedef typename TRAITS_T::comparator_t comparator_t;
   typedef typename TRAITS_T::ignorer_t ignorer_t;
   typedef typename TRAITS_T::leaf_t leaf_t;

   inline explicit jdk_tree( size_t max_items=100 ) : 
   buf( sizeof( leaf_t ) * max_items),
      first_free(0)
   {
      buf.fill();
      clear(0,index_t(max_items));
   }

   inline explicit jdk_tree( const jdk_string_filename &filename ) :
   buf( filename ),
      first_free(0)
   {
      buf.fill();
   }

   inline explicit jdk_tree( const buf_t &otherbuf ) :
   buf( otherbuf ),
      first_free(0)
   {
      buf.fill();	    
   }

   inline ~jdk_tree()
   {
   }

   inline const jdk_tree<TRAITS_T> & operator = ( const jdk_tree<TRAITS_T> &o )
   {
      if( &o != this )
      {
         buf = o.buf;
         buf.fill();		
         first_free = o.first_free;
      }
      return *this;
   }

   inline void clear()
   {
      buf.fill();		
      first_free=0;
   }

   inline void clear( index_t start, index_t end )
   {
      leaf_t *casted_buf = (leaf_t *)buf.get_data();

      for( index_t i=start; i<end; ++i )
      {
         casted_buf[i].clear();	
      }		
   }

   inline size_t count() const
   {
      return buf.get_data_length() / sizeof( leaf_t );
   }

   inline size_t count_items_used() const
   {
      size_t cnt=count();
      size_t last=0;
      leaf_t *casted_buf = (leaf_t *)buf.get_data();

      for( size_t i=0; i<cnt; ++i )
      {
         if( !casted_buf[i].is_free() )
            last=i;
      }
      return last+1;
   }

   inline size_t calc_memory_used() const
   {
      int cnt=count_items_used();
      return cnt * sizeof( leaf_t );
   }


   inline const leaf_t &get( index_t i ) const
   {
      leaf_t *casted_buf = (leaf_t *)buf.get_data();

      return casted_buf[i];
   }

   inline leaf_t &get( index_t i )	
   {
      leaf_t *casted_buf = (leaf_t *)buf.get_data();	
      return casted_buf[i];
   }



   inline index_t add_child( index_t parent_item, const value_t &value, const flags_t &flags )
   {
      index_t item=find_next_free();
      leaf_t *casted_buf = (leaf_t *)buf.get_data();

      // put data into found space, no further siblings, no children
      casted_buf[item].sibling = 0;
      casted_buf[item].child = 0;
      casted_buf[item].value = value;
      casted_buf[item].flags = flags;
      casted_buf[item].parent = parent_item;
      casted_buf[item].allocate();


      // find last child
      if( parent_item!=item && parent_item!=-1 ) // but only if there are any parents at all!
      {
         int last_child = casted_buf[parent_item].child;
         if( last_child )
         {
            while( casted_buf[last_child].sibling )
            {
               last_child = casted_buf[last_child].sibling;
            }

            // found the last child of the parent. Tell him he has a brother
            casted_buf[last_child].sibling = item;		
         }
         else
         {
            // parent has no children yet. add our new child as a child of parent
            if( parent_item!=-1 )
               casted_buf[parent_item].child = item;
         }
      }
      return item;	    
   }

   inline index_t add_sibling( index_t last_sibling_item, const value_t &value, const flags_t &flags )
   {
      index_t item=find_next_free();
      leaf_t *casted_buf = (leaf_t *)buf.get_data();

      // put data into found space, no further siblings, no children
      casted_buf[item].sibling = 0;
      casted_buf[item].child = 0;
      casted_buf[item].value = value;
      casted_buf[item].flags = flags;

      casted_buf[item].parent = casted_buf[last_sibling_item].parent;    // same parent as my older brother
      casted_buf[item].allocate();

      casted_buf[last_sibling_item].sibling=item;
      return item;	    
   }


   inline void remove( index_t i )
   {
      leaf_t *casted_buf = (leaf_t *)buf.get_data();
      casted_buf[i].release();	    
      if( i < first_free )
         first_free=i;
   }

   inline const buf_t &getbuf() const
   {
      return buf;
   }

   inline buf_t &getbuf() 
   {
      return buf;
   }			

   inline index_t find_next_free()
   {
      index_t num = (index_t)(buf.get_data_length() / sizeof(leaf_t));

      for( ;first_free<num; ++first_free )
      {
         if( get(first_free).is_free() )
         {
            break;
         }
      }
      if( first_free==num )
      {
         if( !buf.expand( buf.get_data_length()*3/2 ) )
         {
            JDK_THROW_ERROR("Unable to expand tree buffer", "" );		
         }
         buf.fill();
         clear( first_free, index_t((buf.get_data_length() / sizeof(leaf_t))) );
      }
      return first_free;
   }

   inline bool find_sibling( index_t first_item, const value_t &v, index_t *item ) const
   {
      leaf_t *casted_buf = (leaf_t *)buf.get_data();
      typename TRAITS_T::comparator_t comparator;

      while( !casted_buf[ first_item ].is_free() )
      {
         // does it match?

         if( !casted_buf[first_item].is_end() && comparator( casted_buf[first_item].value, v )==0 )
         {
            // yup! return the value
            *item = first_item;
            return true;
         }
         else
         {
            // nope! check his brother
            if( casted_buf[ first_item ].sibling==0 )
            {
               // no more siblings, so return the last sibling and FALSE
               *item = first_item;
               return false;
            }
            else
            {
               first_item = casted_buf[ first_item ].sibling;			    
            }
         }
      }
      // the caller must see if the item does match. If it doesn't,
      // it is the index of the last sibling.

      return false;
   }

   inline void add( const value_t *list, size_t list_len, const flags_t flags )
   {
      // is there a top family?

      ignorer_t ignore;
      size_t list_pos=0;

      // skip any initial items that are needed to ignore	    
      while( list_pos<list_len && ignore(list[list_pos]) )
         ++list_pos;

      if( get(0).is_free() )
      {
         // nope, this is our first ever add
         index_t i=-1;

         while( list_pos<list_len )
         {
            // add only the items that are not to be ignored
            if( !ignore(list[list_pos]) )
            {
               i=add_child(i,list[list_pos],flags);
            }
            list_pos++;
         }
         i=add_child(i,value_t(0),flags);
         get(i).set_end();

         // and we are done!
         return;		    
      }
      else
      {
         // find a matching char
         index_t i=0;

         while(1)
         {				            
            // skip any items that are needed to ignore	    
            while( list_pos<list_len && ignore(list[list_pos]) )
               ++list_pos;

            if( list_pos==list_len )
            {
               i=add_child(get(i).parent,value_t(0),flags);
               get(i).set_end();

               return;    // we are done!
            }


            if( find_sibling(i,list[list_pos],&i) )
            {
               ++list_pos;

               // We already have it
               // so traverse to the child of this sibling.
               if( get(i).child ==0 )
               {
                  // no child, so add the rest of the children here
                  while( list_pos<list_len )
                  {
                     if( !ignore(list[list_pos]) )
                        i=add_child(i,list[list_pos],flags);
                     list_pos++;
                  }

                  i=add_child(i,value_t(0),flags);
                  get(i).set_end();

                  // and we are done!
                  return;
               }
               else
               {
                  i=get(i).child;
               }
            }
            else   
            {
               // no sibling available

               // i points to last sibling of the family.
               i = add_sibling( i, list[list_pos++], flags );

               while( list_pos<list_len )
               {
                  if( !ignore(list[list_pos]) )
                     i=add_child(i,list[list_pos],flags);
                  list_pos++;
               }

               i=add_child(i,value_t(0),flags);
               get(i).set_end();

               // and we are done!
               return;
            }
         }
      }
   }

   inline bool find_shortest( 
      const value_t *list, 
      size_t list_len, 
      flags_t *flags=0, 
      size_t *match_len=0, 
      index_t *match_item=0
      ) const
   {
      leaf_t *casted_buf = (leaf_t *)buf.get_data();
      index_t i=0;
      index_t list_pos=0;
      ignorer_t ignore;

      // skip any initial items that are needed to ignore	    
      while( list_pos<list_len && ignore(list[list_pos]) )
         ++list_pos;

      while( list_pos<list_len && !casted_buf[i].is_free() )
      {
         if( !find_sibling(i,list[list_pos],&i ) )
         {
            // no match, return false
            return false;
         }

         // is there a child?
         if( casted_buf[i].child )
         {				
            // follow the child.
            i = casted_buf[i].child;

            // if any of the children are an end point, then we have a match

            for( index_t j=i; j!=0; j=casted_buf[j].sibling )
            {
               if( casted_buf[j].is_end() )
               {	
                  // found a match!

                  if( flags )
                     *flags = casted_buf[j].flags;
                  if( match_len )
                     *match_len = list_pos+1;
                  if( match_item )
                     *match_item = j;
                  return true;						
               }					
            }

         }
         else
         {
            // no, so we have a match.
            if( flags )
               *flags = casted_buf[i].flags;
            if( match_len )
               *match_len = list_pos+1;
            if( match_item )
               *match_item = i;
            return true;
         }

         // move to the next non ignored item in the list
         while( list_pos<list_len && ignore(list[++list_pos]) );            
      }

      // no match
      return false;
   }


   inline bool find_longest(
      const value_t *list, 
      size_t list_len, 
      flags_t *flags=0, 
      size_t *match_len=0, 
      index_t *match_item=0,
      index_t initial_leaf_pos=0,
      size_t initial_list_pos=0
      ) const
   {
      leaf_t *casted_buf = (leaf_t *)buf.get_data();
      index_t i=initial_leaf_pos;
      size_t list_pos=initial_list_pos;
      ignorer_t ignore;

      // skip any initial items that are needed to ignore	    
      while( list_pos<list_len && ignore(list[list_pos]) )
         ++list_pos;

      while( list_pos<list_len && !casted_buf[i].is_free() )
      {
         if( !find_sibling(i,list[list_pos],&i ) )
         {
            // no match, return false
            return false;
         }

         // is there a child?
         if( casted_buf[i].child )
         {				
            // follow the child.
            i = casted_buf[i].child;

            // if any of the children are an end point, then we have a match

            for( index_t j=i; j!=0; j=casted_buf[j].sibling )
            {
               if( casted_buf[j].is_end() )
               {	
                  // found a match!

                  // go through all children again and see if we can find a longer match

                  //typename TRAITS_T::comparator_t comparator;

                  for( index_t k=i; k!=0; k=casted_buf[k].sibling )
                  {
                     if( find_longest(
                        list, 
                        list_len, 
                        flags, 
                        match_len, 
                        match_item,
                        k,
                        list_pos+1
                        ) )
                     {
                        // by definition it is longer than the one we have.
                        return true;
                     }
                  }

                  // our match is the longest
                  if( flags )
                     *flags = casted_buf[j].flags;
                  if( match_len )
                     *match_len = list_pos+1;
                  if( match_item )
                     *match_item = j;
                  return true;						
               }					
            }

         }
         else
         {
            // no, so we have a match.
            if( flags )
               *flags = casted_buf[i].flags;
            if( match_len )
               *match_len = list_pos+1;
            if( match_item )
               *match_item = i;
            return true;
         }

         // move to the next non ignored item in the list
         while( list_pos<list_len && ignore(list[++list_pos]) );            
      }

      // no match
      return false;
   }

   inline void perform_censor(
      value_t *list,
      size_t list_len,
      value_t replacement,
      size_t match_pos,
      size_t match_end_pos
      ) const
   {
      size_t list_pos=match_pos;
      ignorer_t ignore;

      // skip any initial items that are needed to ignore	    
      while( list_pos<list_len && list_pos<=match_end_pos )
      {
         if( !ignore(list[list_pos] ) )
         {
            list[list_pos] = replacement;
         }
         ++list_pos;
      }
   }

   inline bool censor_longest(
      value_t *list, 
      size_t list_len, 
      value_t replacement,
      flags_t *flags=0, 
      size_t *match_len=0, 
      index_t *match_item=0,
      index_t initial_leaf_pos=0,
      size_t initial_list_pos=0
      ) const
   {
      leaf_t *casted_buf = (leaf_t *)buf.get_data();
      index_t i=initial_leaf_pos;
      size_t list_pos=initial_list_pos;
      ignorer_t ignore;

      // skip any initial items that are needed to ignore	    
      while( list_pos<list_len && ignore(list[list_pos]) )
         ++list_pos;

      while( list_pos<list_len && !casted_buf[i].is_free() )
      {
         if( !find_sibling(i,list[list_pos],&i ) )
         {
            // no match, return false
            return false;
         }

         // is there a child?
         if( casted_buf[i].child )
         {				
            // follow the child.
            i = casted_buf[i].child;

            // if any of the children are an end point, then we have a match

            for( index_t j=i; j!=0; j=casted_buf[j].sibling )
            {
               if( casted_buf[j].is_end() )
               {	
                  // found a match!

                  // go through all children again and see if we can find a longer match

                  //typename TRAITS_T::comparator_t comparator;

                  for( index_t k=i; k!=0; k=casted_buf[k].sibling )
                  {
                     if( censor_longest(
                        list, 
                        list_len, 
                        replacement,
                        flags, 
                        match_len, 
                        match_item,
                        k,
                        list_pos+1
                        ) )
                     {
                        // by definition it is longer than the one we have.
                        perform_censor( list, list_len, replacement, initial_list_pos, list_pos );
                        return true;
                     }
                  }

                  // our match is the longest
                  if( flags )
                     *flags = casted_buf[j].flags;
                  if( match_len )
                     *match_len = list_pos+1;
                  if( match_item )
                     *match_item = j;
                  perform_censor( list, list_len, replacement, initial_list_pos, list_pos );

                  return true;						
               }					
            }

         }
         else
         {
            // no, so we have a match.
            if( flags )
               *flags = casted_buf[i].flags;
            if( match_len )
               *match_len = list_pos+1;
            if( match_item )
               *match_item = i;
            perform_censor( list, list_len, replacement, initial_list_pos, list_pos );
            return true;
         }

         // move to the next non ignored item in the list
         while( list_pos<list_len && ignore(list[++list_pos]) );            
      }

      // no match
      return false;
   }


   inline size_t extract( 
      value_t *list, 
      size_t max_len, 
      index_t end_leaf_index 
      ) const
   {
      leaf_t *casted_buf = (leaf_t *)buf.get_data();    
      size_t len=0;
      index_t cur_item=end_leaf_index;
      value_t *p = list;		

      if( end_leaf_index<0 )
      {
         return 0;
      }

      if( casted_buf[cur_item].is_end() )
      {
         cur_item = casted_buf[cur_item].parent;	
      }

      if( cur_item<0 )
      {			
         return 0;
      }		

      while( len<max_len )
      {
         *p++ = casted_buf[ cur_item ].value;
         len++;

         if( casted_buf[cur_item].parent==-1 )
         {
            // found top of tree
            // now reverse the order
            if( len>1 )
            {
               for( size_t i=0; i<len/2; ++i )
               {
                  value_t tmp( list[i] );    			    
                  list[i]=list[len-i-1];
                  list[len-i-1]=tmp;					    
               }
            }
            return len;
         }
         cur_item = casted_buf[ cur_item ].parent;
      }
      return 0;
   }

protected:
   buf_t buf;	
   index_t first_free;
};



#endif


