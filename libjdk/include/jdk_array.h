#ifndef _JDK_ARRAY_H
#define _JDK_ARRAY_H

#include "jdk_world.h"


typedef int (*jdk_array_sort_compare_t)(const void *, const void *);


template <class T>
class jdk_array
{
 public:
	typedef T entry_t;

	explicit jdk_array();
	explicit jdk_array( size_t m );
	explicit jdk_array( const jdk_array<T> &o );

	virtual ~jdk_array();

	const jdk_array<T> &operator = (const jdk_array<T> &o );
	
	void clear();
	
	bool grow( size_t new_max_ptrs=0 );

	size_t count() const;
    size_t getmax() const;

	T *get( size_t i );
	const T *get( size_t i ) const;
	bool set( size_t i, T *v );
	void remove( size_t i );
	bool add( T *v );

#if !JDK_IS_VCPP
	void sort();
	void sort( jdk_array_sort_compare_t compare );
#endif
	const T *binary_search( const T &val, jdk_array_sort_compare_t compare ) const;
	const T *binary_search( const T &val ) const;
  
 private:
	size_t max_ptrs;
	size_t first_free;
    T **ptrs;
  

	void validate()
	{
 #if 0
		size_t first_free=0;
		size_t next_used=0;
		size_t i;

		for( i=0; i<max_ptrs; ++i )
		{
			if( ptrs[i]==0 )
			{
				first_free=i;
				break;
			}
		}

		for( ;i<max_ptrs; ++i )
		{
			if( ptrs[i]!=0 )
			{
				next_used=i;
			}
		}
		fprintf( stderr, "set %08lx: first free %d, next_used=%d %08lx\n",
					(long)this, first_free, next_used, (long)ptrs[next_used] );
#endif
	}
	
};


template <class T>
inline jdk_array<T>::jdk_array() 
: max_ptrs(256), first_free(0), ptrs( new T *[ max_ptrs] )
{
	size_t i;
	for( i=0; i<max_ptrs; ++i )
	{
	    ptrs[i]=0;
	}
}

template <class T>
inline jdk_array<T>::jdk_array( size_t m )
: max_ptrs( m==0 ? 1 : m), first_free(0), ptrs( new T *[max_ptrs] )
{
    size_t i;
	for( i=0; i<max_ptrs; ++i )
	{
	    ptrs[i]=0;
	}
	validate();
}

template <class T>
inline jdk_array<T>::jdk_array( const jdk_array<T> &o )
: max_ptrs( o.getmax() ), first_free( o.first_free ), ptrs( new T*[max_ptrs] )
{
	size_t i;

	for( i=0; i<max_ptrs; ++i )
	{
	    ptrs[i]=0;
	}

	for( i=0; i<o.getmax(); ++i )
	{
	    const T *entry = o.get(i);
	    if( entry )
    		ptrs[i] = new T( *entry );
		else
		    ptrs[i] = 0;
	}
	validate();
}


template <class T>
inline jdk_array<T>::~jdk_array()
{
	for( size_t i=0; i<max_ptrs; ++i )
	{
		delete ptrs[i];
		ptrs[i]=0;
	}
	delete [] ptrs;
}

template <class T>
inline const jdk_array<T> & jdk_array<T>::operator = (const jdk_array<T> &o )
{
	if( &o != this )
	{
		// delete all our contents.
		for( size_t i=0; i<max_ptrs; ++i )
		{
			delete ptrs[i];
		}


		// resize our buffer to match
		grow( o.getmax() );
		first_free = o.first_free;

		// now make deep copy duplicates of other array
		for( size_t i=0; i<o.getmax(); ++i )
		{
    	    T *entry = o.get(i);
    	    if( entry )
        		ptrs[i] = new T( *entry );
    		else
    		    ptrs[i] = 0;
		}

	}
	validate();
	return *this;
}



template <class T>
inline void jdk_array<T>::clear()
{
	for( size_t i=0; i<max_ptrs; ++i )
	{
		delete ptrs[i];
		ptrs[i] = 0;
	}
	first_free=0;
	validate();
}

template <class T>
inline bool jdk_array<T>::grow( size_t new_max_ptrs )
{
	bool r=false;

	if( new_max_ptrs==0 )
	  new_max_ptrs = max_ptrs*3/2;	/* grow to 150% */

	if( new_max_ptrs>max_ptrs )
	{
//	fprintf( stderr, "growing jdk_array to %d\n", new_max_ptrs );
		T **new_ptrs = new T*[new_max_ptrs];
		T **old_ptrs = ptrs;

		if( new_ptrs )
		{
			// during the grow operation the object is always in
			// a valid state for reading.

			r=true;
			size_t i;
			for( i=0; i<max_ptrs; ++i )
			{
				new_ptrs[i] = ptrs[i];
			}
			for( ;i<new_max_ptrs; ++i )
			{
			    new_ptrs[i] = 0;
			}

			ptrs = new_ptrs;
			delete [] old_ptrs;
			max_ptrs = new_max_ptrs;
			validate();			
		}
	}
	else
	{
		r=true;	// no need to grow, so return success
	}


	return r;
}

template <class T>
inline size_t jdk_array<T>::getmax() const
{
	return max_ptrs;
}

template <class T>
inline size_t jdk_array<T>::count() const
{
	return max_ptrs;
}


template <class T>
inline T *jdk_array<T>::get( size_t i )
{
	return ptrs[i];
} 

template <class T>
inline const T *jdk_array<T>::get( size_t i ) const 
{
	return ptrs[i];
} 

template <class T>
inline bool jdk_array<T>::set( size_t i, T *v ) 
{
	bool r=true;
	if( i>max_ptrs )
	{
		if( !grow( i ) )
		{
			r=false;
		}
	}
	
	if( r )
	{			
		validate();
	    if( ptrs[i] )
	    {
//			fprintf( stderr, "jdk_array set overwrite %d: %08lx\n", i, (long)ptrs[i] );
	        delete ptrs[i];
	    }
		else
		{
//			fprintf( stderr, "jdk_array set %d: %08lx\n", i, (long)v );			
		}
		
		ptrs[i] = v;
	}
	
	return r;
} 

template <class T>
inline void jdk_array<T>::remove( size_t i )
{
    if( i<max_ptrs )
    {
//		fprintf( stderr, "jdk_array remove %d\n", i );
        delete ptrs[i];
   	    ptrs[i]=0;
	    if( i<first_free )
	    {
	        first_free=i;
	    }
		validate();
    }
}


template <class T>
inline bool jdk_array<T>::add( T *v ) 
{
	bool r=true;

	while(first_free<max_ptrs && ptrs[first_free])
	{
	    first_free++;
	}
			
	if( first_free>=max_ptrs )
	{
		first_free=0;
		while( first_free<max_ptrs && ptrs[first_free]  )
		{
			first_free++;
		}
		if( first_free>=max_ptrs )
		{
		    r=grow();
		}
	}
	
	if( r )
	{
//		fprintf( stderr, "jdk_array add %d: %08lx\n", first_free, (long)v );
		ptrs[first_free++] = v;
		validate();
	}
	
	return r;
} 


template < class T, class F >
int jdk_array_indirect_compare( const void *a_, const void *b_ )
{
    const T *a = *(const T **)a_;
    const T *b = *(const T **)b_;
    int ret=0;
    
    if( a==b )
    {
        ret=0;
    }
    else if( !a )
    {
        ret=1;    // null entries go to end
    }
    else if( !b )
    {
        ret=-1;    // null entries go to end
    }
    else return F(*a,*b);

}


template <class T>
int jdk_array_compare_helper( const void *a_, const void *b_ )
{
    const T *a = *(const T **)a_;
    const T *b = *(const T **)b_;
    int ret=0;
    
    if( a==b )
    {
        ret=0;
    }
    else if( !a )
    {
        ret=1;    // null entries go to end
    }
    else if( !b )
    {
        ret=-1;    // null entries go to end
    }
    else
    if( (*a) < (*b) )
    {
        ret=-1;
    }
    else if( (*a) > (*b) )
    {
        ret=1;
    }
    return ret;
}


#if !JDK_IS_VCPP
template <class T>
inline void jdk_array<T>::sort( jdk_array_sort_compare_t compare )
{
    qsort( ptrs, max_ptrs, sizeof( T * ), compare );
}
#endif

#if !JDK_IS_VCPP
template <class T>
inline void jdk_array<T>::sort()
{
    jdk_array_sort_compare_t compare=jdk_array_compare_helper< T >;
    qsort( ptrs, max_ptrs, sizeof( T * ), compare );
}
#endif

template <class T>
inline const T *jdk_array<T>::binary_search( 
                                    const T &val,  
                                    jdk_array_sort_compare_t compare
				                  ) const
{
    const T *ptrval = &val;

    T **ret1 = (T **)bsearch( &ptrval, ptrs, max_ptrs, sizeof( T * ), compare );

    T *ret = 0;
    
    if( ret1 )    // was one found?
        ret = *ret1;    // yup, dereference it
	
	return ret;
}


template <class T>
const T *jdk_array<T>::binary_search( 
                                    const T &val
				                  ) const
{
    jdk_array_sort_compare_t compare=jdk_array_compare_helper< T >;
    const T *ptrval = &val;

    T **ret1 = (T **)bsearch( &ptrval, ptrs, max_ptrs, sizeof( T * ), compare );

    T *ret = 0;
    
    if( ret1 )    // was one found?
        ret = *ret1;    // yup, dereference it
	
	return ret;
}


#endif
