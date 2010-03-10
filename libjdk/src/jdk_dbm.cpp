#include "jdk_world.h"
#include "jdk_dbm.h"


#if 0

jdk_dbm::jdk_dbm()
  : dbf(0)
{
  
}


jdk_dbm::~jdk_dbm()
{
  close();	
}

bool jdk_dbm::open_read( const char *fname )
{
  close();
  dbf = gdbm_open( (char *)fname, 0, GDBM_READER, 0, 0 );
  return dbf!=0;
}

bool jdk_dbm::open_write( const char *fname )
{
  close();
  dbf = gdbm_open( (char *)fname, 0, GDBM_WRITER, 0, 0 );
  return dbf!=0;
}

bool jdk_dbm::open_write_create( const char *fname, mode_t mode )
{
  close();
  dbf = gdbm_open( (char *)fname, 0, GDBM_WRCREAT, mode, 0 );
  return dbf!=0;
  
}

bool jdk_dbm::open_create( const char *fname, mode_t mode )
{
  close();
  dbf = gdbm_open( (char *)fname, 0, GDBM_NEWDB, mode, 0 );
  return dbf!=0;	
}

void jdk_dbm::close()
{
  if( dbf )
  {
    gdbm_close( dbf );
    dbf=0;
  }	
}


bool jdk_dbm::store_record( const jdk_dbm_datum & key, const jdk_dbm_datum &content )
{
  datum my_key;
  my_key.dptr = key.dptr;
  my_key.dsize = key.dsize;
  
  datum my_content;
  my_content.dptr = content.dptr;
  my_content.dsize = content.dsize;
  
  return gdbm_store( dbf, my_key, my_content, GDBM_REPLACE )==0;
}


bool jdk_dbm::fetch_record( const jdk_dbm_datum & key, jdk_dbm_datum *content )
{
  datum my_key;
  my_key.dptr = key.dptr;
  my_key.dsize = key.dsize;
  
  datum my_content = gdbm_fetch( dbf, my_key );
  
  content->set( &my_content );
  return my_content.dptr !=0;	
}


bool jdk_dbm::delete_record( const jdk_dbm_datum & key )
{
  datum my_key;
  my_key.dptr = key.dptr;
  my_key.dsize = key.dsize;
  
  return gdbm_delete( dbf, my_key )==0;
}


#endif

