#ifndef _JDK_PAIR_H
#define _JDK_PAIR_H

template <class KEYTYPE, class VALUETYPE>
class jdk_pair
{
 public:
	typedef KEYTYPE key_t;
	typedef VALUETYPE value_t;

	jdk_pair() : key(), value() 
	{
	}
	
	jdk_pair( const KEYTYPE &k, const VALUETYPE &v ) : key( k ), value( v ) 
	{	   
//		fprintf( stderr, "jdk_pair constructor 1\n" );
	}


	jdk_pair( const jdk_pair<KEYTYPE,VALUETYPE> &o ) : key( o.key ), value( o.value )
	{
//		fprintf( stderr, "jdk_pair constructor 2\n" );
	}

	
	const jdk_pair<KEYTYPE,VALUETYPE> operator = ( const jdk_pair<KEYTYPE,VALUETYPE> &o )
	{
//		fprintf( stderr, "jdk_pair operator =\n" );
		key = o.key;
		value = o.value;
		
		return *this;
	}
	
    bool operator == ( const jdk_pair<KEYTYPE,VALUETYPE> &o ) const
	{
		return key==o.key;
	}
	
	bool operator < (const jdk_pair<KEYTYPE,VALUETYPE> &o ) const
	{
		return key<o.key;
	}		
	
	KEYTYPE key;
	VALUETYPE value;
};

template <class KEYTYPE, class VALUETYPE>
inline int operator < ( const jdk_pair<KEYTYPE,VALUETYPE> &a, const jdk_pair<KEYTYPE,VALUETYPE> &b )
{
    return a.key < b.key;
}

template <class KEYTYPE, class VALUETYPE>
inline int operator <= ( const jdk_pair<KEYTYPE,VALUETYPE> &a, const jdk_pair<KEYTYPE,VALUETYPE> &b )
{
    return a.key <= b.key;
}

template <class KEYTYPE, class VALUETYPE>
inline int operator == ( const jdk_pair<KEYTYPE,VALUETYPE> &a, const jdk_pair<KEYTYPE,VALUETYPE> &b )
{
    return a.key == b.key;
}

template <class KEYTYPE, class VALUETYPE>
inline int operator >= ( const jdk_pair<KEYTYPE,VALUETYPE> &a, const jdk_pair<KEYTYPE,VALUETYPE> &b )
{
    return a.key >= b.key;
}

template <class KEYTYPE, class VALUETYPE>
inline int operator > ( const jdk_pair<KEYTYPE,VALUETYPE> &a, const jdk_pair<KEYTYPE,VALUETYPE> &b )
{
    return a.key > b.key;
}


#endif
