#include "jdk_world.h"
#include "jdk_http.h"
#include "jdk_log.h"

const char * jdk_http_request_header::http_methods[] =
{
  "UNKNOWN",
  "ACL",
  "BASELINE-CONTROL",
  "BIND",
  "CHECKIN",
  "CHECKOUT",
  "CONNECT",
  "COPY",
  "DELETE",
  "GET",
  "HEAD",
  "LABEL",
  "LINK",
  "LOCK",
  "MERGE",
  "MKACTIVITY",
  "MKCOL",
  "MKREDIRECTREF",
  "MKWORKSPACE",
  "MOVE",
  "OPTIONS",
  "ORDERPATCH",
  "PATCH",
  "POST",
  "PROPFIND",
  "PROPPATCH",
  "PUT",
  "REBIND",
  "REPORT",
  "SEARCH",
  "TRACE",
  "UNBIND",
  "UNCHECKOUT",
  "UNLINK",
  "UNLOCK",
  "UPDATE",
  "UPDATEREDIRECTREF",
  "VERSION-CONTROL"
};

void jdk_http_request_header::clear() 
{
  map.clear();
  splitter.clear();
  valid=false;
  request_type = REQUEST_UNKNOWN;
  request_type_extended.clear();

  line_count=0;
  url.clear();
  http_version.cpy("HTTP/1.0");
}


bool jdk_http_request_header::parse_buffer( const char *buf, size_t len )
{
  // return true if an entire http request header is parsed or if a parsing error occurred.
  
  splitter.set_buf( (char *)buf, len );
  
  jdk_str<4096> line;    
  while( splitter.scan_for_line(&line) )
  {
    // found a line!	    
    jdk_log( JDK_LOG_DEBUG4, "jdk_http_request_header::parsing line: '%s'", line.c_str() );
    
    // ignore any initial blank lines.
    if( line.is_clear() && line_count==0 )
    {
      continue;
    }
    
    ++line_count;
    
    // is this our final blank line?
    if( line.is_clear() && line_count>0 )
    {			
      // yup, we are done parsing.
      
      // if we have a valid host name then we have a valid request.
      if( !url.host.is_clear() )
      {
        valid=true;
      }
      jdk_log( JDK_LOG_DEBUG4, "request_header parsed.  buf_pos=%d, buf_len=%d", get_buf_pos(), splitter.get_buf_len() );
      return true;
    }
    
    // is it our first line with the request?
    if( line_count==1 )
    {
      if( !parse_first_line(line) )
      {
        // error parsing first line. return true to stop parsing and is_valid() will be false.
        return true;
      }
      
    }
    else
    {
      // ok, it wasnt the first line. check for Host: and Port: lines
      if( line.nicmp( "Host:", 5 )==0 )
      {
        if( url.host.is_clear() )
        {
          // extract the value of this key
          jdk_str<300> tmp;
          tmp.cpy( line.c_str()+6 );
          tmp.strip_begws();
          tmp.strip_endws();
          
          // is there a : specifying port as well?
          char *p = tmp.chr(':');
          if( p )
          {
            // yup, lets grab it and remove it from the string.
            url.port = strtol(p+1,0,10);
            *p='\0';
          }
          url.host = tmp;
        }
      }
      else // how about a Port: line?
        if( line.nicmp( "Port:", 5 )==0 )
        {
          url.port = strtol( line.c_str()+6, 0, 10 );
        }
        else // we dont need to parse this line, just store it.
        {
          // extract the key, up to first colon
          key_t k;
          int pos = line.extract_token(0,&k,":");
          
          // extract the value, from where key ended, skiping all white space
          value_t v;
          v.cpy( line.c_str() + pos + 1 );
          // remove any trailing white space
          v.strip_begws();
          v.strip_endws();
          k.cat(':'); // add the colon back
          k.lower(); // lowercase the key since it is case insensitive
          // add the key and value to our map
          map.add( k, v );
        }
    }
  }
  
  // return false so the caller knows to grab a new buffer
  // and call us again
  //jdk_log( JDK_LOG_DEBUG4, "jdk_http_request_header parsing not finished yet" );
  return false;
}

bool jdk_http_request_header::parse_first_line(const jdk_string &line)
{
  request_type = MAX_REQUEST;
  for( int i=0; i<MAX_REQUEST; ++i )
  {
    if( line.nicmp(http_methods[i],strlen(http_methods[i]))==0 )
    {
      request_type = request_t(i);
      break;
    } 
  }
  
  if( request_type==MAX_REQUEST )
  {
    jdk_log( JDK_LOG_DEBUG3, "jdk_http_request_header is not recognized: %s - allowing anyways", line.c_str() );
    request_type_extended=line;
  }
  
  // ok, now extract the url or dir/file from the request line
  
  jdk_str<4096> file;
  
  const char *first_space = line.chr(' ');
  if( first_space )
  {
    file.cpy( first_space+1 );
  }
  else
  {
    jdk_log( JDK_LOG_DEBUG3, "invalid first line of http request: %s", line.c_str() );
  }
  
  // now terminate the string at the first blank space to grab and crop out HTTP/x.y version info
  
  char *p = file.chr(' ');
  if(p)
  {
    http_version.cpy( p+1 );
    http_version.strip_endws();
    *p='\0';
  }
  
  // ok, does this text start with 'http:', 'https:' or contain a ':' ?
  
//    if( file.ncmp( "http:", 5 )==0 || file.ncmp("https:",6)==0 )
  if( file.chr(':') && file.get(0)!='/' )
  {
    // yup, it is a full URL already!
    url.explode( file );
    
    jdk_log_debug2( "exploded url: %s %s : %d %s", url.protocol.c_str(), url.host.c_str(), url.port, url.path.c_str() );
  }
  else
  {
    // ok, it is just a partial url. we still need the Host: and Port: lines coming later.
    url.path.cpy( file );
    url.host.clear();
    url.protocol = "http";
  }
  
  
  return true;    // successfully parsed
}

bool jdk_http_request_header::flatten( jdk_dynbuf &dest, bool full_url ) const
{
  if( is_valid())
  {
    jdk_str<4096> line;
    dest.clear();
    
    if( get_request_type()==REQUEST_UNKNOWN )
    {
      line.cpy( request_type_extended );
    }
    else
    {
      line.cpy( http_methods[ get_request_type() ] );
      line.cat( " " );
      
      if( get_request_type()==REQUEST_CONNECT )
      {
        jdk_str<256> hostport;
        
        hostport.form( "%s:%d", url.host.c_str(), url.port );
        
        line.cat( hostport );
      }
      else
      {
        if( full_url )
        {
          line.cat( url.unexplode() );
        }
        else
        {
          line.cat( url.path );
        }
      }
		
      line.cat( " " );
      line.cat( http_version );
      line.cat( "\r\n" );
    }

    dest.append_from_string( line );
    
    int p = url.port;
    if( p==-1 )
      p=80;
    
    if( p==80 )
    {
      line.form( "Host: %s\r\n", url.host.c_str() );
    }
    else
    {
      line.form( "Host: %s:%d\r\n", url.host.c_str(), p );
    }
    
    
    dest.append_from_string( line );
    
    // now append all other keys and values
    
    for( size_t i=0; i<map.count(); ++i )
    {
      const pair_t *entry = map.get( i );
      
      // is it a valid entry?
      if( entry )
      {
        // create the line for this key value pair
        line.form( "%s %s\r\n", entry->key.c_str(), entry->value.c_str() );
        
        // only append lines that arent just full of white space
        if( line.len()>3 )
        {
          dest.append_from_string( line );
        }
      } 
    }
    
    // thats it, finish it off with a blank line
    dest.append_from_string( "\r\n" );
    return true;
  }
  else
  {
    return false;
  }
}


bool jdk_http_request_header::has_entry( const jdk_http_request_header::key_t &k ) const
{
  return map.find( k, 0 )>=0;
}


int jdk_http_request_header::find_entry( 
  const jdk_http_request_header::key_t &k, 
  const jdk_http_request_header::value_t **v,
  int start_pos
  ) const
{
  int pos = map.find( k, start_pos );
  if( pos>=0 && v)
  {
    *v = &map.getvalue(pos);
  }
  
  return pos;
}

void jdk_http_request_header::delete_entry( const jdk_http_request_header::key_t &k )
{
  int pos=0;
  
  while( (pos=map.find(k,pos))>=0 )
  {
    map.remove(pos);
  }
}

bool jdk_http_request_header::set_entry( const jdk_http_request_header::key_t &k, const jdk_http_request_header::value_t &v )
{
  delete_entry(k);
  return map.add( k, v );
}

bool jdk_http_request_header::add_entry( const jdk_http_request_header::key_t &k, const jdk_http_request_header::value_t &v )
{
  return map.add( k, v );
}


//////
void jdk_http_response_header::clear() 
{
  map.clear();
  splitter.clear();
  valid=false;
  line_count=0;
  http_version.clear();
  http_response_code=0;
  http_response_text.clear();
}


bool jdk_http_response_header::parse_buffer( const char *buf, size_t len )
{
  // return true if an entire http request header is parsed or if a parsing error occurred.
  
  splitter.set_buf( (char *)buf, len );
  
  jdk_str<4096> line;    
  while( splitter.scan_for_line(&line) )
  {
    // found a line!	    
    jdk_log( JDK_LOG_DEBUG4, "jdk_http_response_header::parsing line: '%s'", line.c_str() );
    
    // an initial blank line means the server is fucked and didnt respond with the proper http header.
    if( line.is_clear() && line_count==0 )
    {
      valid=true;
      return true;
    }
    
    ++line_count;
    
    // is this our final blank line?	    
    if( line.is_clear() )
    {
      // yup, we are done parsing.
      
      // if we have a valid http version and code then it is ok
      
      if( !http_version.is_clear() && http_response_code!=0 )
      {
        valid=true;
      }
      
      return true;
    }
    
    // is the server fucked and did not include a blank line before putting a '<' from an html file?
    
    if( !line.is_clear() && line.get(0)=='<' )
    {
      // yup!
      
      // yup, we are done parsing.
      
      valid=true;
      
      return true;				
    }
    
    
    
    // is it our first line with the request?
    if( line_count==1 )
    {
      if( !parse_first_line(line) )
      {
        // error parsing first line. return true to stop parsing and is_valid() will be false.
        return true;
      }
      
    }
    else
    {
      // we dont need to parse this line, just store it.
      // extract the key, up to the first ':'
      key_t k;
      int pos = line.extract_token(0,&k,":");
      k.lower();
      
      // extract the value, from where key ended, skiping the colon and the white space
      value_t v;
      v.cpy( line.c_str() + pos + 1 );
      // remove any trailing white space
      v.strip_begws();
      v.strip_endws();
      
      // put the colon back
      k.cat(':');
      // add the key and value to our map
      map.add( k, v );
    }
  }
  
  
  // return false so the caller knows to grab a new buffer
  // and call us again
  //jdk_log( JDK_LOG_DEBUG4, "jdk_http_response_header parsing not finished yet" );
  return false;
}

bool jdk_http_response_header::parse_first_line(const jdk_string &line)
{
  //if( line.nicmp( "HTTP", 4 )!=0 && line.nicmp( "ICY",3)!=0 )
  //{
  //    jdk_log( JDK_LOG_INFO, "Response header is not 'HTTP or ICY': %s", line.c_str() );
  //    return false;    // cant understand request
  //}
  
  // ok, now extract the http version and response code and response text
  
  int pos = line.extract_token( 0, &http_version, " \t" );
  char *p=0;
  
  http_response_code = strtol( line.c_str() + pos + 1, &p, 10 );
  
  if( p )
  {
    http_response_text.cpy( p );
  }
  
  jdk_log( JDK_LOG_DEBUG2, "response is version '%s', code %d, text '%s'", 
           http_version.c_str(), http_response_code, http_response_text.c_str()
    );
  
  return true;    // successfully parsed
}

bool jdk_http_response_header::flatten( jdk_dynbuf &dest ) const
{
  if( is_valid() )
  {
    jdk_str<4096> line;
    dest.clear();
    line.form( 
      "%s %d %s\r\n", 
      http_version.c_str(), 
      http_response_code,
      http_response_text.c_str()
      );
    
    dest.append_from_string( line );
    
    
    // append the "Host:" entry first if there is one
    {		
      int num = map.find( key_t("Host:") );;
      if( num!=-1 )
      {
        line.form( "%s %s\r\n", map.get(num)->key.c_str(), map.get(num)->value.c_str() );
        dest.append_from_string( line );
      }						
    }
    
    // now copy all other entries
    for( size_t j=0; j<map.count(); ++j )
    {
      const pair_t *entry = map.get( j );
      
      // is it a valid entry?
      if( entry && entry->key.len()>1 && entry->key.icmp("Host:")!=0)
      {
        // create the line for this key value pair
        line.form( "%s %s\r\n", entry->key.c_str(), entry->value.c_str() );
        
        // only append lines that arent just full of white space
        if( line.len()>3 )
        {
          dest.append_from_string( line );
        }
      } 
    }
    
    // thats it, finish it off with a blank line
    dest.append_from_string( "\r\n" );
    return true;
  }
  else
  {
    return false;
  }
}

bool jdk_http_response_header::has_entry( const jdk_http_response_header::key_t &k ) const
{
  return map.find( k, 0 )>=0;
}


int jdk_http_response_header::find_entry( 
  const jdk_http_response_header::key_t &k, 
  const jdk_http_response_header::value_t **v,
  int start_pos
  ) const
{
  int pos = map.find( k, start_pos );
  if( pos>=0 )
  {
    *v = &map.getvalue(pos);
  }
  
  return pos;
}

void jdk_http_response_header::delete_entry( const jdk_http_response_header::key_t &k )
{
  int pos=0;
  
  while( (pos=map.find(k,pos))>=0 )
  {
    map.remove(pos);
  }
}

bool jdk_http_response_header::set_entry( const jdk_http_response_header::key_t &k, const jdk_http_response_header::value_t &v )
{
  delete_entry(k);
  return map.add( k, v );
}

bool jdk_http_response_header::add_entry( const jdk_http_response_header::key_t &k, const jdk_http_response_header::value_t &v )
{
  return map.add( k, v );
}

bool jdk_http_response_header::add_entry( const char *k, const char *v )
{
  return map.add( jdk_http_response_header::key_t(k), jdk_http_response_header::value_t(v) );
}




