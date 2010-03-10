#include "jdk_world.h"
#include "jdk_util.h"
#include "jdk_cgisettings.h"


bool jdk_cgi_loadsettings( 
  jdk_settings *settings, 
  const char *cgidata
  ) 
{
  jdk_setting s;
  
  const char *p = cgidata;
  size_t orig_len = strlen(p);
  size_t cgidata_len = orig_len;
  
  settings->clear();
  
  while( (p=jdk_cgi_extract_pair(
            p, cgidata_len,
            s.key.c_str(), s.key.getmaxlen(),
            s.value.c_str(), s.value.getmaxlen()
            ))!=0 
    )
  {
    // got a new setting.  Add it to the settings map
    
    if( s.key.len()>0 )
    {   		
      settings->set( s );
    }
    
    
    // calculate length of remaining string
    cgidata_len = orig_len - (p-cgidata);
  }
  
  return true;
} 

bool jdk_cgi_loadsettings( 
  jdk_settings *settings, 
  const jdk_dynbuf &cgidata_ 
  ) 
{
  jdk_dynbuf cgidata( cgidata_ );
  char ender=0;
  cgidata.append_from_data( &ender, 1 );
  
  jdk_setting s;
  
  const char *p = (char *)cgidata.get_data();
  size_t cgidata_len = cgidata.get_data_length();
  
//	jdk_log_debug3( "buffer length=%d", cgidata_len );
//	fprintf( stderr, "%s", cgidata.get_data() );
  //jdk_log_debug3( "full cgi: %s", cgidata.get_data() );
  
  s.value.resize(cgidata_len);
  
  settings->clear();
  
  while( (p=jdk_cgi_extract_pair(
            p, cgidata_len,
            s.key.c_str(), s.key.getmaxlen(),
            s.value.c_str(), s.value.getmaxlen()
            ))!=0 
    )
  {
    // got a new setting.  Add it to the settings map
    
    if( s.key.len()>0 )
    {			
      settings->set( s );
//			jdk_log_debug3( "added %s=%s", s.key.c_str(), s.value.c_str() ); 
    }
    
    // recalc cgidata_len 
    cgidata_len = cgidata.get_data_length() - (p-(char *)cgidata.get_data());
  }
  
  return true;
  
} 


bool jdk_cgi_savesettings( 
  const jdk_settings &settings, 
  char *cgidata, 
  size_t maxlen 
  ) 
{
  jdk_str<3000> line_buf;
  char *p=cgidata;
  size_t len=0;
  
  size_t num=settings.count();
  
  bool first=true;
  for( size_t i=0; i<num; ++i )
  {
    const jdk_setting *s = settings.get(i);
    if( s )
    {
      line_buf.clear();
      
      jdk_cgi_encode_pair(
        s->key.c_str(),
        s->value.c_str(),
        line_buf.c_str(), line_buf.getmaxlen()
        );
      
      size_t line_buf_len = line_buf.len();
      if( line_buf_len>0 )
      {
        if( first )
        {
          first=false;
        }
        else
        {
          // put & before next entry, only if this isnt the first entry
          *p++ = '&';
          ++len;
        }      
      }

      if( len+line_buf_len < maxlen-1 )
      {
        jdk_strcpy( p, line_buf.c_str() );
        
        p+=line_buf_len;
        len+=line_buf_len;              
      }
      else
      {
        return false;
      }
    }
  }
  *p='\0';
  return true;
} 


bool jdk_cgi_savesettings( 
  const jdk_settings &settings, 
  jdk_dynbuf *cgidata 
  ) 
{
  jdk_str<3000> line_buf;
  
  size_t num=settings.count();
  cgidata->clear();
  
  bool first=true;

  for( size_t i=0; i<num; ++i )
  {
    const jdk_setting *s = settings.get(i);
    
    if( s )
    {      
      jdk_cgi_encode_pair(
        s->key,
        s->value,
        line_buf
        );
      
      
      if( line_buf.len()>0 )
      {
        if( !first )
        {
          first=false;
        }
        else
        {
          cgidata->append_from_string( "&" );
        }
        cgidata->append_from_string( line_buf.c_str() );
      }
    }
  }
  
  return true;	
} 



