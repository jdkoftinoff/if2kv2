#include "jdk_world.h"
#include "jdk_remote_buf.h"
#include "jdk_log.h"
#include "jdk_http.h"
#include "jdk_httprequest.h"
#include "jdk_lz.h"

volatile bool jdk_remote_buf::remote_enabled = true;


#if 0
jdk_remote_buf::jdk_remote_buf( 
  const jdk_settings &settings,
  const jdk_str<256> setting_field_prefix,
  long max_len_,
  bool ignore_remote_if_local_exists_							   
  ) 
  :
  remote_url(),
  local_file(),
  local_timestamp_file(),
  proxy(),
  max_len( max_len_ ),	  
  remote_is_compressed(false),
  local_is_compressed(false),
  buf(),
  timestamp(),
  ignore_remote_if_local_exists( ignore_remote_if_local_exists_ ),
  local_exists( false )
{
  jdk_string_url tmp_remote_url;
  
  local_file = settings.get_prefixed( setting_field_prefix, "_url_cache" );
  
  tmp_remote_url = settings.get_prefixed( setting_field_prefix, "_url" );
  
  proxy = settings.get( "further_proxy" );
  
  load_local();
  
  if( !tmp_remote_url.is_clear() )
  {
    size_t urllen=tmp_remote_url.len();
    if( urllen>4 && strcmp( tmp_remote_url.c_str() + urllen-4, ".zip" )==0 )
    {
      remote_is_compressed=true;
      jdk_log_debug3( "remote file %s is compressed\n", tmp_remote_url.c_str() );
    }
    remote_url = tmp_remote_url;
  }
  
}
#endif

#if 0
jdk_remote_buf::jdk_remote_buf(
  const jdk_string_filename &local_file_,
  const jdk_string_url &remote_url_,
  const jdk_string_url &proxy_,
  long max_len_,
  bool ignore_remote_if_local_exists_
  )
  :
  remote_url(),
  local_file( local_file_ ),
  local_timestamp_file(),
  proxy( proxy_ ),
  max_len( max_len_ ),
  remote_is_compressed(false),	
  local_is_compressed(false),
  buf(),
  timestamp(),
  ignore_remote_if_local_exists( ignore_remote_if_local_exists_ ),
  local_exists(false)
{
  load_local();
  if( !remote_url_.is_clear() )
  {
    size_t urllen=remote_url.len();
    if( urllen>5 && strcmp( remote_url.c_str() + urllen-4, ".zip" )==0 )
    {
      remote_is_compressed=true;
      jdk_log_debug3( "remote file %s is compressed\n", remote_url.c_str() );
    }
    
    remote_url=remote_url_;
  }
}
#endif


jdk_remote_buf::jdk_remote_buf(
  const jdk_string_filename &local_file_,
  const jdk_string_url &remote_url_,
  const jdk_string_url &proxy_,
  long max_len_,
  bool ignore_remote_if_local_exists_,
  const jdk_string_url &proxy_username_,
  const jdk_string_url &proxy_password_,
  bool mark_as_hidden_
  )
  :
  remote_url(),
  local_file( local_file_ ),
  local_timestamp_file(),
  proxy( proxy_ ),
  proxy_username( proxy_username_ ),
  proxy_password( proxy_password_ ),
  mark_as_hidden( mark_as_hidden_ ),
  max_len( max_len_ ),
  remote_is_compressed(false),	
  local_is_compressed(false),
  buf(),
  timestamp(),
  ignore_remote_if_local_exists( ignore_remote_if_local_exists_ ),
  local_exists(false)  
{
  load_local();
  if( !remote_url_.is_clear() )
  {
    size_t urllen=remote_url.len();
    if( urllen>5 && strcmp( remote_url.c_str() + urllen-4, ".zip" )==0 )
    {
      remote_is_compressed=true;
      jdk_log_debug3( "remote file %s is compressed\n", remote_url.c_str() );
    }
    
    remote_url=remote_url_;
  }
}


void jdk_remote_buf::clear_buf()    // without erasing timestamp
{
  buf.clear();
}

bool jdk_remote_buf::load_local()
{
  if( !local_file.is_clear() )
  {
    size_t len=local_file.len();
    if( len>5 && strcmp( local_file.c_str() + len-4, ".zip" )==0 )
    {
      local_is_compressed=true;
      jdk_log_debug3( "file %s is compressed", local_file.c_str() );
    }
    
    local_timestamp_file = local_file;
    local_timestamp_file.cat( ".date" );
    
    // try load the initial files
    if( local_is_compressed )
    {
      jdk_dynbuf tmpbuf;
      if( tmpbuf.append_from_file( local_file ) )
      {				
        jdk_log_debug3( "decompresssing %s", local_file.c_str() );				
        jdk_lz_decompress( &tmpbuf, &buf );
        local_exists=true;
      }
      else
      {
        local_exists=false;	
      }
      
    }			
    else
    {
      buf.clear();
      if( buf.append_from_file( local_file ) )
      {
        local_exists=true;	
      }			
      else
      {
        local_exists=false;	
      }			
    }
    timestamp.append_from_file( local_timestamp_file );						
  }
  if( local_exists && mark_as_hidden )
  {
    mark_file_hidden( local_file );
    mark_file_hidden( local_timestamp_file );
  }
  return local_exists;
}


bool jdk_remote_buf::check_and_grab()
{
  load_local();			

  if( !remote_enabled )
  {
    return false;
  }

  if( ignore_remote_if_local_exists && local_exists)
  {
    return true;
  }
  

#if 0
#if JDK_IS_WIN32
  if( !jdk_internet_is_connected() )
    return false;
#endif

#endif
  if( check() )
  {
    return grab();
  }
  return false;
}

bool jdk_remote_buf::check()
{

#if 0
#if JDK_IS_WIN32
  if( !jdk_internet_is_connected() )
    return false;
#endif
#endif
  if( !remote_enabled )
  {
    return false;
  }

  
  if( remote_url.is_clear() )
    return false;   // not updated because there is no remote file
  
  // do a http head to see if the last-modified time has changed
  
  bool ret=false;
  
  jdk_http_response_header response_header;
  
  if( jdk_http_head(
        remote_url.c_str(),
        &response_header,
        proxy.c_str(),
        false,
        0,
        proxy_username.c_str(),
        proxy_password.c_str()
        )==200 )
  {
    // ok, got valid response from server. Is the last-modified field different from last time?
    const jdk_http_response_header::value_t *value;
    if( response_header.find_entry("Last-modified:",&value) !=-1 )
    {
      
      // is it the same as the data in timestamp? if it is the same, then set ret to false
      ret = (value->nicmp( (const char *)timestamp.get_data(), value->len() )!=0);
      
    }
    
    if( ret )
    {
      jdk_log_debug1( "Found %s is newer than '%s'", remote_url.c_str(), local_file.c_str() );
    }
    else
    {
      jdk_log_debug2( "Found %s is not newer than '%s'", remote_url.c_str(), local_file.c_str() );
    }
  }
  else
  {
    ret=false;   
  }
  
  return ret;
}

bool jdk_remote_buf::grab()
{

#if 0
#if JDK_IS_WIN32
  if( !jdk_internet_is_connected() )
    return false;
#endif
#endif
  if( !remote_enabled )
  {
    return false;
  }
  // try to grab the real file.
  if( remote_url.is_clear() )
    return false; // no file to grab!
  
  jdk_http_response_header response_header;
  
  jdk_dynbuf tmpbuf;
  
  if( jdk_http_get(
        remote_url.c_str(),
        &tmpbuf,
        max_len,
        &response_header,
        proxy.c_str(),
        false,
        0,
        proxy_username.c_str(),
        proxy_password.c_str()
        )==200 )
  {
    // got valid response from server and entire file.
    // clear our existing buffer, and copy it over, uncompressing if the file ends in '.lz'
    buf.clear();
    
    if( remote_is_compressed )
    {
      jdk_log_debug3( "remote %s is compressed", remote_url.c_str() );
      jdk_lz_decompress( &tmpbuf, &buf );	
      jdk_log_debug3( "was %d bytes, uncompressed to %d",
                      tmpbuf.get_data_length(),
                      buf.get_data_length() );
    }
    else
    {				
      buf.append_from_buf( tmpbuf );
      jdk_log_debug3( "remote %s is not compressed", remote_url.c_str() );
    }
    
    jdk_log_debug1( "Grabbed %s", remote_url.c_str() );
    
    // store last-modified time into timestamp buffer
    const jdk_http_response_header::value_t *value;
    if( response_header.find_entry("Last-modified:",&value) !=-1 )
    {
      timestamp.clear();
      timestamp.append_from_string( *value );
      jdk_log_debug2( "Date of new %s is '%s'", remote_url.c_str(), value->c_str() );
    }
    else
    {
      jdk_log_debug1( "Didn't have Last-modified field" );
    }
    
    
    // write buf to our local file if we have a local file name
    
    if( !local_file.is_clear() )
    {
      bool r=false;
      
      if( mark_as_hidden )
      {
        mark_file_unhidden( local_file );
      }

      if( local_is_compressed && remote_is_compressed )
      {
        // it is already compressed for us in tmpbuf
        jdk_log_debug3( "storing pre-compressed file to %s", local_file.c_str() );
        r=tmpbuf.extract_to_file(local_file);
      }
      else if( local_is_compressed && !remote_is_compressed )
      {
        // we have to compress it.
        jdk_log_debug3( "compressing and storing file to %s", local_file.c_str() );
        tmpbuf.clear();
        jdk_lz_compress( &buf, &tmpbuf );
        r=tmpbuf.extract_to_file(local_file);
      }
      else if( !local_is_compressed )
      {
        // we dont compress it
        jdk_log_debug3( "storing uncompressed file to %s", local_file.c_str() );					
        r=buf.extract_to_file(local_file);	
      }
      
      
      if( r )
      {
        jdk_log_debug2( "Wrote new %s to '%s'", remote_url.c_str(), local_file.c_str() );
        
        // success writing file, now try write timestamp file
        if( mark_as_hidden )
        {
          mark_file_unhidden( local_timestamp_file );
        }
        
        if( !timestamp.extract_to_file( local_timestamp_file ) )
        {
          jdk_log_debug2( "Couldn't write '%s'", local_timestamp_file.c_str() );
        }
        if( mark_as_hidden )
        {
          mark_file_hidden( local_file );
          mark_file_hidden( local_timestamp_file );
        }
        
        // we ignore the return code because we may not have write permissions
        // for the file. doesn't matter, we have the local info anyways.
      }
      else
      {
        jdk_log_debug2( "Couldn't write to %s", local_file.c_str() );
      }
    }
    else
    {
      jdk_log_debug2( "No file to write %s to", remote_url.c_str() );
    }
    
    return true;    // got a new file in buf!
  }
  return false;
}


void jdk_remote_buf::mark_file_hidden( const jdk_string_filename &fname )
{
#if JDK_IS_WIN32
  BOOL r=SetFileAttributesA( fname.c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM );
  if( !r )
    jdk_log_debug3( "Error setting file '%s' to hidden/system attribs", fname.c_str() );
#else
#warning no mark_file_unhidden on platform
#endif
}


void jdk_remote_buf::mark_file_unhidden( const jdk_string_filename &fname )
{
#if JDK_IS_WIN32
  BOOL r=SetFileAttributesA( fname.c_str(), FILE_ATTRIBUTE_NORMAL );
  if( !r )
    jdk_log_debug3( "Error setting file '%s' to normal attribs", fname.c_str() );
#else
#warning no mark_file_unhidden on platform
#endif
}
                                        