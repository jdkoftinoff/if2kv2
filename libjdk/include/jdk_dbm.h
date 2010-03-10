#ifndef _JDK_DBM_H
#define _JDK_DBM_H

#include "jdk_platform.h"

#if 0
#include <gdbm.h>


class jdk_dbm_datum : public datum
{
public:
	jdk_dbm_datum()
	{
		dptr = 0;
		dsize = 0;
	}
	
	jdk_dbm_datum( const jdk_dbm_datum &o )
	{
		if( o.dptr && o.dsize )
		{
			dptr = (char *)malloc( o.dsize );
			dsize = o.dsize;
			memcpy( dptr, o.dptr, dsize );			
		}
		else
		{
			dptr = 0;
			dsize = 0;
		}
				
	}
	
	~jdk_dbm_datum()
	{
		release();	
	}
	
	
	const jdk_dbm_datum & operator = ( const jdk_dbm_datum &o )
	{
		release();
		
		if( o.dptr && o.dsize )
		{
			dptr = (char *)malloc( o.dsize );
			dsize = o.dsize;
			memcpy( dptr, o.dptr, dsize );
		}
		else
		{
			dptr = 0;
			dsize = 0;
		}		
		return *this;
	}

	bool valid() const
	{
		return dptr!=0;	
	}
	
	
	void set( datum *d )
	{
		release();
		dptr = d->dptr;
		dsize= d->dsize;
	}
	
	void resize( int len )
	{
		dptr = (char *)realloc( dptr, len );
	   	dsize = len;		
	}
	

	void set_string( const char *text )
	{
		release();
		dsize = strlen(text);
		dptr = (char *)malloc(dsize);
		memcpy( dptr, text, dsize );
	}
	
	void extract_string( char *text ) const
	{
	  	if( dptr )
		{
			memcpy( text, dptr, dsize );
			text[dsize]='\0';
		}
		else
		{
			*text='\0';	
		}		
		
	}	
	
	
	void release()
	{
		if( dptr )
		{
			free( dptr );
			dptr=0;
		}
		dsize=0;		
	}
	
};



class jdk_dbm
{
public:
	friend class jdk_dbm_iterator;
	
	jdk_dbm();
	~jdk_dbm();

	bool open_read( const char *fname );
	bool open_write( const char *fname );	
	bool open_write_create( const char *fname, mode_t mode=0666 );
	bool open_create( const char *fname, mode_t mode=0666 );	
	void close();
	
	bool store_record( const jdk_dbm_datum & key, const jdk_dbm_datum &content );
	bool fetch_record( const jdk_dbm_datum & key, jdk_dbm_datum *content );
	bool delete_record( const jdk_dbm_datum & key );
	
private:
	jdk_dbm( const jdk_dbm & );
	const jdk_dbm & operator = ( const jdk_dbm & );
	
	GDBM_FILE dbf;	
};

class jdk_dbm_iterator
{
public:
	jdk_dbm_iterator( jdk_dbm &db_ ) 
	  :
		db( db_ ),
		current_key()
	{
	}
	
	~jdk_dbm_iterator()
	{
	}
	

	bool begin()
	{
		datum tmp;
		tmp = gdbm_firstkey( db.dbf );
		current_key.set(&tmp);
		return tmp.dptr!=0;
	}
	
	bool next()
	{
		datum tmp;
		tmp = gdbm_nextkey( db.dbf, current_key );
		current_key.set(&tmp);
		return tmp.dptr!=0;
	}
	

    const jdk_dbm_datum &current() const
	{
		return current_key;	
	}
	
	
private:
	jdk_dbm &db;
	jdk_dbm_datum current_key;
};


#endif

#endif
