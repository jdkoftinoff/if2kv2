#ifndef _JDK_ATOM_H
#define _JDK_ATOM_H



template <class VALUE>
struct jdk_atom
{
	typedef VALUE value_t; 

	inline jdk_atom()
	{
		contents = 0;
		next = 0;
	}
	
	inline jdk_atom( const jdk_atom<VALUE> &other )
	  : value(other.value), contents(0), next(0)
	{
		if( other.contents )
			contents = new jdk_atom( *other.contents );
		if( other.next )
		  	next = new jdk_atom( *other.next );
	}
	
	inline jdk_atom( VALUE v, jdk_atom<VALUE> *contents_=0, jdk_atom<VALUE> *next_ = 0 )
	  :
		value(v),
		contents(contents_),
		next(next_)
	{
	}
	
	inline ~jdk_atom()
	{
		delete next;
		delete contents;
	}

	inline const jdk_atom<VALUE> &operator = ( const jdk_atom<VALUE> &other )
	{
		if( &other!=this )
   		{
			value = other.value;
			if( other.contents )
				contents = new jdk_atom( *other.contents );
			if( other.next )
			  	next = new jdk_atom( *other.next );   			
		}		
		return *this;
	}
	
	
	inline jdk_atom<VALUE> *get_last()
	{
		jdk_atom<VALUE> *cur=this;
		while( cur->next )
		{
			cur=cur->next;
		}
		return cur;
	}
	
   	inline jdk_atom<VALUE> * add_next( jdk_atom<VALUE> *o )
	{
		jdk_atom<VALUE> *cur=this;
		while( cur->next )
		{
			cur=cur->next;	
		}
		cur->next = o;
		return o;
	}
	
	inline jdk_atom<VALUE> * add_contents( jdk_atom<VALUE> *o )
	{
		if( contents )
		{
			jdk_atom<VALUE> *cur=contents;
			while( cur->next )
			{
				cur=cur->next;	
			}
			cur->next = o;
		}
		else
		{
		  	contents=o;	
		}		
		return o;
	}
	
	inline void remove_contents()
	{
		delete contents;
		contents=0;
	}
	
	inline void remove_next()
	{
		if( next && next->next )
		{			
			jdk_atom<VALUE> *new_next=next->next;
			next->next=0;
			delete next;
			next=new_next;
		}
		else
		{
			delete next;
			next=0;
		}					  
	}	
	
	
	VALUE value;
	jdk_atom<VALUE> *contents;
	jdk_atom<VALUE> *next;
};

template <class T>
inline jdk_atom<T> *jdk_atom_new( T v, jdk_atom<T> *contents=0, jdk_atom<T> *next=0 )
{
	return new jdk_atom<T>( v, contents, next );
}



#endif
