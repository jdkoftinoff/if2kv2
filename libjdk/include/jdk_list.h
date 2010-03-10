#ifndef _JDK_LIST_H
# define _JDK_LIST_H

template <class T> 
class jdk_list
{
 public:
	explicit jdk_list( int max_num_ );
	explicit jdk_list( const jdk_list<T> & );
	const jdk_list<T> &operator = ( const jdk_list<T> & );
		
	~jdk_list();
	
	bool add( T *item );
	
	int find( const T &item ) const;
	
	void remove( int num );
	
	void clear();
		
	int getnum() const
	{
		return max_num;
	}
	
	int count() const
	{
	    return max_num;
	}
	
	
	T *get( int num )
	{
		return buf[num];
	}
	
	const T *get( int num ) const
	{
		return buf[num];
	}
		
private:
  	T **buf;
	int max_num;
	int first_free;
};

template <class T>
jdk_list<T>::jdk_list( int max_num_ ) 
    : 
    buf( new T *[max_num_]), 
    max_num( max_num_ ), 
    first_free(0)
{
	for( int i=0; i<max_num; ++i )
	{
		buf[i] = 0;
	}
	
}

template <class T>
jdk_list<T>::jdk_list( const jdk_list &o )
    :
    buf( new T *[o.max_num]), 
    max_num( o.max_num ),
    first_free( o.first_free )
{
    for( int i=0; i<max_num; ++i )
    {
        if( o.buf[i] )
	    {
            buf[i] = new T(*o.buf[i]);
	    }
	    else
	    {
	        buf[i] = 0;
		}
    }
}

template <class T>
jdk_list<T>::~jdk_list()
{
	for( int i=0; i<max_num; ++i )
	{
		delete buf[i];	
	}		
	delete [] buf;
}

template <class T>
const jdk_list<T> &jdk_list<T>::operator =( const jdk_list<T> &o )
{
    clear();
    delete [] buf;
    buf = new T *[o.max_num];
    max_num = o.max_num;
    first_free = o.first_free;

    for( int i=0; i<max_num; ++i )
    {
        if( o.buf[i] )
	    {
            buf[i] = new T(*o.buf[i]);
	    }
	    else
	    {
	        buf[i]=0;
		}
    }
    return *this;
}

template <class T>
void jdk_list<T>::clear()
{
	for( int i=0; i<max_num; ++i )
	{
		delete buf[i];	
		buf[i]=0;
	}		
	first_free=0;
}


template <class T>	
bool jdk_list<T>::add( T *item )
{
	// assume failure
	int r = -1;
	
	// do we have a free item at all?
	if( ((first_free>=0) && (first_free<max_num)) )
	{
		// yep this will work.
		r = first_free;
		
		// enter the item.
		buf[first_free] = item;
		
		// find the next free item
		for( first_free=first_free+1; first_free<max_num; ++first_free )
		{				
			if( buf[first_free]==0 )
			  break; // found it!
		}
		
		// if we didnt find it first_free with be == max_num
	}
	
	return r!=-1;
}


template <class T>
int jdk_list<T>::find( const T &item ) const
{
	int r=-1;
	for( int i=0; i<max_num; ++i )
	{
		if( *buf[i] == item )
		{
			r=i;
			break;
		}			
	}
	
	return r;
}

template <class T>	
void jdk_list<T>::remove( int num )
{		
	if( buf[num] )
	{	
		delete buf[num];
		buf[num]=0;
		if( num<first_free )
		{	
			first_free=num;
		}		
	}
}
	
	

#endif
