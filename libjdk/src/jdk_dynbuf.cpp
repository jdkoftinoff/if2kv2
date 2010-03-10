/* -*- mode: C; mode: fold; -*- */

#include "jdk_world.h"
#include "jdk_dynbuf.h"

void jdk_dynbuf::markup()
{
  if( buf_len - data_len>0 )
  {
    memset( data+data_len, 0xaa, buf_len-data_len );
  }
}

bool jdk_dynbuf::resize( size_t len ) 
{
  if( len==0 )
    len=1;
  unsigned char *new_data=new unsigned char[len];
  if( new_data )
  {
    size_t minlen=len;
    if( buf_len<len ) minlen=buf_len;
    
    memcpy( new_data, data, minlen );
    
    unsigned char *old_data = data;
    data=new_data;
    buf_len=len;
    delete [] old_data;		
    return true;
  }
  else
  {
    return false;	
  }	   
}



bool jdk_dynbuf::expand( size_t len ) 
{
  if( len>buf_len )
  {
    return resize( len );
  }
  return true;
}



bool jdk_dynbuf::shrink( size_t len )
{
  if( len<buf_len )
  {
    return resize(len);
  }
  return true;
}




