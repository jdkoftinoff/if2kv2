#ifndef _JDK_POOL_H
#define _JDK_POOL_H

template <typename OBJ,typename ID, typename USAGE >
class jdk_pool
{
public:
	typedef OBJ obj_t;
	typedef ID id_t;
	typedef USAGE usage_t;

	inline explicit jdk_pool( const jdk_pool<obj_t,id_t,usage_t> &o )
	  :
		obj_mem( new obj_t[ o.size ] ),
		usage_mem( new usage_t[ o.size ] ),
		size( o.size ),
		first_available( o.first_available ),
		free_on_destruct(true)
	{
		for( id_t i=0; i<size; ++i )
		{
			obj_mem[i] = o.obj_mem[i];
			usage_mem[i] = o.usage_mem[i];
		}
	}
	
	
	inline explicit jdk_pool( id_t size_ )
	  : 
		obj_mem(new obj_t[size]),
		usage_mem( new usage_t[size] ),
		size(size_),
		first_available(0),
		free_on_destruct(true)
	{
	}	

	inline explicit jdk_pool( obj_t *obj_mem_, usage_t *usage_mem_, id_t size_ )
	  :
		obj_mem( obj_mem_ ),
		usage_mem( usage_mem_ ),
		size(size_),
		first_available(0),
		free_on_destruct(false)
	{
		for( id_t i=0; i<size; i++ )
		{
			if( usage_mem[i]==0 )
			{
				first_available=i;
				break;
			}			
		}
		
	}
	
    inline ~jdk_pool()
	{
		if( free_on_destruct )
		{
			delete [] usage_mem;
			delete [] obj_mem;
		}		
	}
	
	inline void resize( id_t new_size )
	{
		if( free_on_destruct )
		{			
			obj_t *new_obj = new obj_t [new_size];
			usage_t *new_usage = new usage_t [new_size];
			
			id_t num_to_copy = size;
			if( new_size < size )
			  num_to_copy = new_size;
			
			for( id_t i=0; i<num_to_copy; ++i )
			{
				new_obj[i] = obj_mem[i];
				new_usage[i] = usage_mem[i];
			}
			delete [] obj_mem;
			delete [] usage_mem;
			obj_mem = new_obj;
			usage_mem = new_usage;
		}
		
	}
	

	inline id_t find_or_allocate( const obj_t &o )
	{
		id_t i = find( o );
		if( i!=-1 )
		{
			inc_usage( i );
		}
		else
		{
			i = allocate( o );	
		}
		return i;
	}
	
	
	inline id_t allocate( const obj_t &o )
	{
		if( first_available>size )
		{
			resize( size*2 );
		}
		
		for( id_t i=first_available; i<size; i++ )
		{
			if( usage_mem[i]==0 )
			{
				obj_mem[i] = o;
				usage_mem[i]=1;
				first_available=i+1;
				return i;
			}			
		}
		return id_t(-1);
	}
	
	inline void release( const id_t &id )
	{
		usage_mem[ id ] = 0;
		if( id < first_available )
		  	first_available = id;
	}
	
	
	inline obj_t *get( const id_t &id )
	{
		return &obj_mem[id];	
	}
	
	inline const obj_t *get( const id_t &id ) const
	{
		return &obj_mem[id];	
	}
	
	inline id_t find( const obj_t &o ) const
	{
		for( id_t i=0; i<size; ++i )
		{
			if( usage_mem[i]>0 )
			{
				if( obj_mem[i] == o )
				{
					return i;
				}
			}
		}
		return id_t(-1);
	}
	
	
	inline bool inc_usage( const id_t &id )
	{
		if( ++usage_mem[id]==1 )
		{
			if( id == first_available )
			{
				first_available = id+1;
			}
			
		}
		return usage_mem[id]==1;
	}
	
	inline bool dec_usage( const id_t &id )
	{
		if( --usage_mem[id]==0 )
		{
			if( id < first_available )
			{
				first_available=id;	
			}
			
		}
		return usage_mem[id]==0;
		  
	}

	inline usage_t get_usage( const id_t &id ) const
	{
		return usage_mem[id];
	}
	
	inline id_t get_size() const
	{
		return size;
	}
	
private:
	obj_t *obj_mem;
	usage_t *usage_mem;
	
	id_t size;
	id_t first_available;
	bool free_on_destruct;
	
	void operator = ( const jdk_pool<obj_t,id_t,usage_t> &o ); // ILLEGAL	
};



#endif
