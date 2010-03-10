#include "jdk_world.h"
#include "jdk_settings.h"

#if JDK_IS_WIN32
bool jdk_settings_win32registry::load_registry( HKEY area, const char *keyname )
{   
  HKEY key;
  DWORD dummy;
  DWORD e;
  
  if( (e=RegCreateKeyExA(
         area,
         keyname,
         0,
         "",
         0,
         KEY_READ | KEY_QUERY_VALUE,
         0,
         &key,
         &dummy
         ))==0 )
  {		   
    DWORD index=0;
    
    while( 1 )
    {    	  		   
      key_t subkey_name;
      DWORD len=DWORD(subkey_name.getmaxlen());
      DWORD type=0;
      value_t subkey_value_string;
      DWORD value_len=DWORD(subkey_value_string.getmaxlen());
      
      e=RegEnumValueA(
        key,
        index++,
        subkey_name.c_str(),
        &len,
        0,
        &type,
        (BYTE *)subkey_value_string.c_str(),
        &value_len
        );
      
      if( e==ERROR_NO_MORE_ITEMS )
        break;
      
      if( type==REG_SZ )
      {
        value_t unescaped_value;
        jdk_string_unescape( subkey_value_string, unescaped_value );
        
        set( subkey_name, unescaped_value );
      }
    }
# if 0		   
    if( e!=0 )
    {
      char e1[256];
      sprintf( e1, "%ld", e );
      MessageBox( 0, e1, "error", MB_OK );
      LPVOID buf;
      FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
        0, 
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
        (LPTSTR)&buf,0,NULL );
      MessageBox( 0, (char*)buf, "error", MB_OK );
      LocalFree(buf);
    }
# endif		   
    RegCloseKey(key);
    
    return true;
  }
  return false;
  
}


bool jdk_settings_win32registry::save_registry( HKEY area, const char *keyname ) const
{     
  size_t num=count();
  for( size_t i=0; i<num; ++i )
  {
    const pair_t *p=get(i);
    if( p )
    {
      value_t escaped_value;
      jdk_string_escape( p->value, escaped_value );
      if( !jdk_util_registry_set_string( area, keyname, p->key.c_str(), escaped_value.c_str() ) )
        return false;
    }
  }
  return true;
}


#endif

#if JDK_IS_MACOSX
#include <CoreFoundation/CoreFoundation.h>

bool jdk_settings_macosx_plist::set_args( 
  const jdk_setting_description *default_settings, 
  int argc, char **argv, 
  const char *config_file 
  )
{ 
  set_defaults( default_settings );
  
  if( config_file )
  {
    load_plist( config_file );
  }
  
  for( int i=1; i<argc; ++i )
  {
    if( argv[i][0] == '-' && argv[i][1] == '-' )
    {
      // was it a request for help?
      
      if( jdk_stricmp( argv[i], "--help" )==0 )
      {
        // yes, dump the list of default settings and descriptions to stdout
        jdk_setting_description_dump( default_settings, stdout );
        
        // and exit.
        exit(1);
      }
      
      char *p = strchr((char *)argv[i],'=');
      
      jdk_setting s;
      
      if( p )
      {
        // found an = sign
        // everything between -- and = is the key name
        s.key.ncpy( &argv[i][2], p-&argv[i][2]+1 );
        // everything after is the key value
        s.value.cpy( p+1 );
      }
      else
      {
        // no = sign, just set the key name to a blank value
        s.key.cpy( &argv[i][2] );
      }
      
      set( s );
    }
    else
    {
      if( !load_plist( argv[i] ) )
      {
        jdk_log( JDK_LOG_ERROR, "Unable to load configuration file '%s'\n", argv[i] );
      }
    }
  }
  return true;
}

bool jdk_settings_macosx_plist::load_plist( const char *fname )
{
  bool r=false;        
  SInt32 errorCode=0;
  
  CFStringRef cf_fname = CFStringCreateWithCString(0,fname,kCFStringEncodingMacRoman);
  
  CFURLRef fileURL=0;
  fileURL = CFURLCreateWithFileSystemPath(
    0,            	
    cf_fname,
    kCFURLPOSIXPathStyle,				
    false 
    );
  
  CFDataRef resourceData=0; 
  r = CFURLCreateDataAndPropertiesFromResource(
    0,
    fileURL,
    &resourceData,		// place to put file data
    NULL,		
    NULL,
    &errorCode
    );
  
  if( r && errorCode==0 )
  {
    CFDictionaryRef	dict=0;
    
    // Reconstitute the dictionary using the XML data.
    dict = (CFDictionaryRef)CFPropertyListCreateFromXMLData( 
      0,
      resourceData,
      kCFPropertyListImmutable,
      0
      );
    
    if( dict )
    {
      CFIndex dictcount=CFDictionaryGetCount( dict );
      
      if( dictcount>0 )
      {
        const void **keys = new const void *[dictcount];
        const void **values = new const void *[dictcount];
        
        CFDictionaryGetKeysAndValues(
          dict,
          keys,
          values
          );
        
        for( int i=0; i<dictcount; ++i )
        {
          key_t key;
          if( CFStringGetCString( (CFStringRef)keys[i], key.c_str(), key.getmaxlen(), kCFStringEncodingMacRoman ))
          {
            value_t value;
            if( CFStringGetCString( (CFStringRef)values[i], value.c_str(), value.getmaxlen(), kCFStringEncodingMacRoman ))
            {
              set( key, value );
            }
          }                         
        }
        r=true;
        
        delete [] keys;
        delete [] values;
      }
      CFRelease(dict);
    }		
  }
  if( resourceData )
    CFRelease(resourceData);	
  CFRelease(fileURL);
  CFRelease(cf_fname);	
  return r;
}

bool jdk_settings_macosx_plist::save_plist( const char *fname ) const
{   
  bool r=false;
  
  CFMutableDictionaryRef dict;
  
  dict = CFDictionaryCreateMutable( 0, 0, 
                                    &kCFTypeDictionaryKeyCallBacks,
                                    &kCFTypeDictionaryValueCallBacks 
    );
  
  int num=count();
  int i;
  for( i=0; i<num; ++i )
  {
    const pair_t *p=get(i);
    if( p )
    {
      CFStringRef key = CFStringCreateWithCString(0,p->key.c_str(),kCFStringEncodingMacRoman);
      CFStringRef value = CFStringCreateWithCString(0,p->value.c_str(),kCFStringEncodingMacRoman);
      
      CFDictionarySetValue( dict, key, value );
      CFRelease(key);
      CFRelease(value);
    }
  }
  
  if( i==num )
  {
    SInt32 errorCode;
    CFDataRef xmlData;
    xmlData = CFPropertyListCreateXMLData( 0, dict );
    
    CFStringRef cf_fname = CFStringCreateWithCString(0,fname,kCFStringEncodingMacRoman);
    
    
    CFURLRef fileURL;            
    fileURL = CFURLCreateWithFileSystemPath(0,            	
                                            cf_fname,
                                            kCFURLPOSIXPathStyle,				
                                            false 
      );
    CFRelease(cf_fname);
    
    r = CFURLWriteDataAndPropertiesToResource (
      fileURL,		// URL to use
      xmlData,		// data to write
      NULL,	
      &errorCode
      );				
    CFRelease( fileURL );
    CFRelease( xmlData );        
  }
  
  CFRelease(dict);
  
  return r;       
}


#endif



void jdk_setting_description_dump( const jdk_setting_description *d, FILE *f )
{
  fprintf( f, "Configuration settings:\n\n" );
  while( d && d->name )
  {
    fprintf( f, "\"%s\":\n   default = \"%s\"\n   %s\n\n",
             d->name, d->default_value, d->description ? d->description : "" );
    ++d;
  }
}



jdk_settings::jdk_settings(
  const jdk_setting_description *default_settings, 
  int argc, 
  char **argv, 
  const char *config_file 
  )
: map(), dummy_value()
{
  set_args( default_settings, argc, argv, config_file );
}



bool jdk_settings::set_args( 
  const jdk_setting_description *default_settings, 
  int argc, char **argv, 
  const char *config_file
  )
{  
  if( default_settings )
  {
    set_defaults( default_settings );
  }
  
  if( config_file )
  {
    load_file( config_file );
  }
  
  for( int i=1; i<argc; ++i )
  {
    if( argv[i][0] == '-' && argv[i][1] == '-' )
    {
      // was it a request for help?
      
      if(  default_settings && jdk_stricmp( argv[i], "--help" )==0 )
      {
        // yes, dump the list of default settings and descriptions to stdout
        jdk_setting_description_dump( default_settings, stdout );
        
        // and exit.
        exit(1);
      }
      
      char *p = strchr((char *)argv[i],'=');
      
      jdk_setting s;
      
      if( p )
      {
        // found an = sign
        // everything between -- and = is the key name
        s.key.ncpy( &argv[i][2], p-&argv[i][2]+1 );
        // everything after is the key value
        s.value.cpy( p+1 );
      }
      else
      {
        // no = sign, just set the key name to a blank value
        s.key.cpy( &argv[i][2] );
      }
      
      set( s );
    }
    else
    {
      if( !load_file( argv[i] ) )
      {
        jdk_log( JDK_LOG_ERROR, "Unable to load configuration file '%s'\n", argv[i] );
      }
    }
  }
  return true;
}

bool jdk_settings::parse_line( const jdk_string &line_buf )   
{		    
  if( line_buf.len() > 0 && line_buf.get(0)!='#' && line_buf.get(0)!=';' )
  {
    jdk_setting_key key;
    jdk_setting_value value;
    
    size_t pos=0;
    
    key.clear();
    value.clear();
    
    // extract key from line
    pos = line_buf.extract_token( pos, &key, " \t=" );
    
    // fail on error
    if( pos<0 )
    {
      return false;
    }
    
    // extract rest of the line as the value, skipping initial ws
    pos++;
    pos+= line_buf.spn( pos, " \t" );
    
    value.cpy( line_buf.getaddr(pos) );
    
    // if the value was starting with a quote then zero out the last quote
    
    if( value.get(0)=='"' )
    {
      value.cpy( value.getaddr(1) );			
      char *q = value.rchr('"');
      if( q )
      {
        *q='\0';
      }
    }
    else
    {
      // otherwise zero out a comment start.
      char *q;
      
      q = value.chr(';');
      
      if( q )
      {
        *q='\0';
      }
      
      q = value.chr('#');
      
      if( q )
      {
        *q='\0';
      }
      
      value.strip_endws();
    }
    
    
    
    // if the value starts with '#' or ';' then it is a comment,
    // there is no value - clear it
    
    if( value.get(0) == '#' || value.get(0) == ';' )
    {
      value.clear();
    }
    
    
    // is it a valid key?
    
    if( key.len()>0 && key.get(0)!='#' && key.get(0)!=';' )
    {
      // got a full setting. add it to our map
      
      value_t unescaped;
      jdk_string_unescape( value, unescaped );
      return set( key,unescaped );
    }
    
  }
  return false;
}

bool jdk_settings::load_file( FILE *f )
{    
  jdk_str<4096> line_buf;
  
  while( !feof(f) && !ferror(f) )
  {
    if( jdk_read_string_line( &line_buf, f ) )
    {
      parse_line( line_buf );
    }
  }
  
  return !ferror(f);
}

bool jdk_settings::load_buf( const jdk_dynbuf &buf )
{
  int bufpos=0;
  jdk_str<4096> line_buf;
  
  while( bufpos<(int)buf.get_data_length() && bufpos>=0 )
  {
    bufpos = buf.extract_to_string( line_buf.c_str(), bufpos, line_buf.getmaxlen() );
    
    if( bufpos!=-1 )
    {
      parse_line( line_buf );
    }   
  }
  
  return true;
}

bool jdk_settings::load_file( const char *fname )
{  
  bool r=false;
  FILE *f = jdk_fopen( fname, "rt" );
  if( f )
  {
    r=load_file( f );
    fclose(f);
  }
  
  return r;
} 

bool jdk_settings::save_file( 
  FILE *f, 
  const char *line_end, 
  bool use_quotes 
  ) const 
{
  if( ferror(f) )
  {
    return false;
  }
  
  jdk_str<1500> line_buf;
  
  size_t num=map.count();
  
  for( size_t i=0; i<num; ++i )
  {
    const jdk_setting *s;
    
    s = map.get( i );
    if( s )
    {
      line_buf.clear();
      
      line_buf.cat( s->key );
      
      line_buf.cat("=" );
      if( use_quotes )
        line_buf.cat( "\"" );
      
      value_t escaped;
      jdk_string_escape( s->value, escaped );
      line_buf.cat( escaped );
      
      if( use_quotes )
        line_buf.cat( "\"" );
      
      if( line_end )
      {
        line_buf.cat( line_end );
      }
      
      fprintf( f, "%s", line_buf.c_str() );
    }
  }
  
  return !ferror(f);
  
}

bool jdk_settings::save_buf(
  jdk_dynbuf *buf,
  const char *line_end,
  bool use_quotes
  ) const
{  
  jdk_str<1500> line_buf;
  
  size_t num=map.count();
  
  for( size_t i=0; i<num; ++i )
  {
    const jdk_setting *s;
    
    s = map.get( i );
    
    if( s )
    {
      line_buf.clear();
      
      line_buf.cat( s->key );
      
      line_buf.cat("=" );
      
      if( use_quotes )
        line_buf.cat( "\"" );
      
      value_t escaped;
      jdk_string_escape( s->value, escaped );
      line_buf.cat( escaped );
      
      if( use_quotes )
        line_buf.cat( "\"" );
      
      buf->append_from_string( line_buf.c_str() );
      
      if( line_end )
      {
        buf->append_from_string( line_end );
      }
    }
  }
  
  return true;
} 

bool jdk_settings::merge( const jdk_settings &s )
{
  if( &s!=this )
  {
    for( size_t i=0; i<s.count(); ++i )
    {
      const pair_t *a = s.get(i);
      
      if( a )
      {
        erase( a->key );
        if( !set(*a) )
          return false;
      }
    }
  }
  return true;
}

bool jdk_settings::save_file( 
  const char *fname,
  const char *line_end,
  bool use_quotes
  ) const 
{
  bool r=false;
  FILE *f = jdk_fopen( fname, "wt" );
  
  if( f )
  {
    r=save_file( f, line_end, use_quotes );
    fclose(f);    
  }
  
  return r;
} 

bool jdk_settings::set_defaults( 
  const jdk_setting_description *default_settings 
  )
{
  const jdk_setting_description *d = default_settings;
  
  // if we have any default settings, set them now
  while( d && d->name)
  {
    if( !set( key_t(d->name), value_t( d->default_value ) ) )
    {
      JDK_THROW_ERROR( "Unable to set setting", d->name );
      return false;
    }
    
    ++d;
  }
  return true;
}

void jdk_settings::expand_string( 
  const char *p, 
  jdk_string &out 
  ) const
{
  key_t varname;
  
  enum
    {
      in_text,
      in_dollar,
      in_brace
    } state = in_text;
  
  for( ; *p; ++p )
  {
    switch( state )
    {
    case in_text:	// in normal text, look for dollar sign
    {
      if( *p=='$' )	// found it! go into dollar mode
      {
        state = in_dollar;
      }
      else
      {
        out.cat( *p ); // was not a dollar sign so copy it over
      }
      break;
    }
    
    case in_dollar: // in dollar mode, look for another dollar or open brace
    {
      if( *p=='$' )	// a double dollar sign means print a single dollar sign
      {
        state = in_text;
        out.cat( *p );
      }
      else if( *p=='{' || *p=='(' ) // a brace means clear our variable name and go into brace mode
      {
        varname.clear();
        state = in_brace;
      }
      else
      {
        // otherwise we assume this is an error, just put a single $ and go back into text mode
        state = in_text; 
        out.cat( *p );
      }
      break;
    }
    
    case in_brace:
    {
      // when we are in a brace, look for an end of brace character.
      if( *p=='}' || *p==')' )
      {
        // found it! replace the ${xxx} with the value of the setting xxx
        out.cat( get(varname) );
        state = in_text;	// and go back to text mode
      }
      else
      {
        // not at end of variable name yet so just keep appending chars to the varname
        varname.cat( *p );
      }
      break;
    }
    }
  }
}







