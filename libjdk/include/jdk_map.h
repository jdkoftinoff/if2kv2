#ifndef _JDK_MAP_H
#define _JDK_MAP_H

#include "jdk_array.h"
#include "jdk_valarray.h"
#include "jdk_pair.h"

template <class KEYTYPE, class VALUETYPE>
class jdk_map
{
 public:
	typedef KEYTYPE key_t;
	typedef VALUETYPE value_t;
	typedef jdk_pair<KEYTYPE,VALUETYPE> pair_t;

	jdk_map() : array()
	{
	}
   
	explicit jdk_map( size_t m ) : array(m)
	{
	}
	
	explicit jdk_map( const jdk_map &o ) : array(o.array)
	{
	}	
	
	virtual ~jdk_map()
	{
	}
	
	void clear()
	{
	    array.clear();
	}

    size_t count() const
    {
        return array.count();
    }
    
    jdk_pair<KEYTYPE,VALUETYPE> *get(size_t i )
    {
        return array.get(i);
    }

    const jdk_pair<KEYTYPE,VALUETYPE> *get(size_t i ) const
    {
        return array.get(i);
    }
        
	VALUETYPE &getvalue( size_t i )
	{
		return array.get(i)->value;
	}
	
	const VALUETYPE &getvalue( size_t i ) const
	{
		return array.get(i)->value;
	}

    bool add( const KEYTYPE &key, const VALUETYPE &value )
    {
		jdk_pair<KEYTYPE,VALUETYPE> *entry = new jdk_pair<KEYTYPE,VALUETYPE>(key,value);
        return array.add( entry );        
    }

    bool add( jdk_pair<KEYTYPE,VALUETYPE> *entry )
    {
        return array.add( entry );        
    }
	
	bool set( const KEYTYPE &key, const VALUETYPE &value, bool case_sensitive=false )
	{
		bool r=false;
		int pos;
		
//		fprintf(stderr,"About to create pair\n");
		jdk_pair<KEYTYPE,VALUETYPE> *entry = new jdk_pair<KEYTYPE,VALUETYPE>(key,value);
//		fprintf(stderr,"pair created at %08lx\n", (long)entry );
		// do we already have this key?
		pos=find(key,0,case_sensitive);
		if( pos==-1 )
		{			
			// no, we don't so add it.
			r = array.add( entry );
		}
		else
		{
		    // change our existing key
		    r = array.set( pos, entry );
		}
		return r;
	}
	
	void remove( size_t i )
	{
	    array.remove(i);
	}
	
	void remove( const KEYTYPE &key, bool case_sensitive=false )
	{
	    int pos=0;
	    while( (pos=find( key, pos, case_sensitive))!=-1 )
	    {
	        remove(pos);
	    }
	}
	

	int find( const KEYTYPE &k, size_t start_pos=0, bool case_sensitive=false ) const
	{		
		size_t num = count();
		int r = -1;
		
		for( size_t i=start_pos; i<num; ++i )
		{
			const jdk_pair<KEYTYPE,VALUETYPE> *p = get(i);
			
			if( p )
			{
			    if( case_sensitive )
			    {
    				if( p->key == k )
	    			{
		    			r=int(i);
			       		break;
				    }
				}
				else
				{
    				if( p->key.icmp(k)==0 )
	    			{
		    			r=int(i);
			       		break;
				    }				
				}
			}
			
		}
		
		return r;
	}
	
	
private:
    jdk_array< jdk_pair<KEYTYPE,VALUETYPE> > array;
    
};

#endif



