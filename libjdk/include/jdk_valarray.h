#ifndef _JDK_VALARRAY_H
#define _JDK_VALARRAY_H

#include "jdk_world.h"



template <class T>
class jdk_valarray
{
 public:
	typedef T entry_t;

   jdk_valarray(size_t n=1024) : N(n)
    {
		used=new bool[N];
		vals=new T[N];
		
		for( size_t i=0; i<N; ++i )
		{
			used[i]=false;
		}
    }
    jdk_valarray( const jdk_valarray<T> &o ) : N(o.getmax())
    {
		used=new bool[N];
		vals=new T[N];

		for( size_t i=0; i<N; ++i )
		{
			const T *v = o.get(i);
			if( v )
			{
				set(i,*v);
			}
		}
    }

	virtual ~jdk_valarray()
    {
		delete [] used;
		delete [] vals;
    }

	const jdk_valarray<T> &operator = (const jdk_valarray<T> &o )
    {
		delete [] used;
		delete [] vals;
		N=o.getmax();
		used=new bool[N];
		vals=new T[N];
		for( size_t i=0; i<N; ++i )
		{
			const T *v = o.get(i);
			if( v )
			{
				set(i,*v);
			}
		}
		return *this;
    }
	
	void clear()
    {
		for( size_t i=0; i<N; ++i )
		{
			used[i]=false;
		}		
    }
	
	size_t count() const { return N; }
    size_t getmax() const { return N; }

	T *get( size_t i )
    {
		T *v=0;
		if( used[i] )
			v=&vals[i];
		return v;
    }
	const T *get( size_t i ) const
    {
		const T *v=0;
		if( used[i] )
			v=&vals[i];
		return v; 
    }
	bool set( size_t i, T &v )
    {
		if( i<N && i>=0 )
		{
			vals[i]=v;
			used[i]=true;
		}
		return i<N && i>=0;
    }

    bool set( size_t i, const T &v )
    {
		if( i<N && i>=0 )
		{
			vals[i]=v;
			used[i]=true;
		}
		return i<N && i>=0;
    }
   
	void remove( size_t i )
    {
		used[i]=false;
    }
   
	bool add( T &v )
    {
		bool r=false;
		for( size_t i=0; i<N; ++i )
		{
			if( used[i]==false )
			{
				vals[i]=v;
				used[i]=true;
				r=true;
				break;
			}
		}
		return r;
    }

   	bool add( const T &v )
    {
		bool r=false;
		for( size_t i=0; i<N; ++i )
		{
			if( used[i]==false )
			{
				vals[i]=v;
				used[i]=true;
				r=true;
				break;
			}
		}
		return r;
    }
  

 private:
    size_t N;
	T *vals;
    bool *used;
};




#endif
