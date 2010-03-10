#include "jdk_world.h"
#include "jdk_http_server.h"
#include "jdk_bindir.h"

jdk_http_server_worker::jdk_http_server_worker( jdk_http_server_generator_list &generators_ ) : 
  generators( generators_ ), 
  request(),
  request_buf(8192), 
  response(),
  response_flattened(4096), 
  response_buf(16384),
  tmp_buf_len()
{
}

jdk_http_server_worker::~jdk_http_server_worker()
{
}

void jdk_http_server_worker::handle_request( jdk_client_socket &sock )
{  
  request.clear();
  request_buf.clear();
  
  jdk_str<1024> connection_id;
  sock.get_remote_addr( connection_id.c_str(), connection_id.getmaxlen() );
  {
    // kill port on incoming address
    char *p=connection_id.chr(':');
    if( p )
      *p=0;
  }
  
  
  if( read_request(sock) )
  {
    response.clear();
    response_buf.clear();
    
    for( int i=0; i<generators.getnum(); ++i )
    {
      jdk_http_server_generator *gen = generators.get(i);
      if( gen )
      {
        if( gen->handle_request(
              request,
              request_buf,
              response,
              response_buf,
              connection_id
              ) )
        {
          break;	// found the appropriate handler!
        }																
      }			
    }	
    
    if( response.is_valid() )
    {
      jdk_log( JDK_LOG_DEBUG2, "Generator request handled" );				
      if( !send_response(sock) )
      {
        jdk_log( JDK_LOG_DEBUG1, "Error sending response" );
      }				
    }
    else
    {
      jdk_log( JDK_LOG_DEBUG2, "Generator not found for request" );					
      if( !send_error( sock, 404 ) )
      {
        jdk_log( JDK_LOG_DEBUG1, "Error sending error response" );	
      }				
    }						
  }
  
}

bool jdk_http_server_worker::read_request( jdk_client_socket &sock )
{
  request.clear();
  request_buf.clear();
  while( sock.is_open_for_reading() )
  {
    tmp_buf_len = sock.read_data( tmp_buf, sizeof(tmp_buf)-1 );
    if( tmp_buf_len>0 )
    {
      if( request.parse_buffer( tmp_buf, tmp_buf_len ) )
      {
        jdk_log_debug1( "parsed request" );
        
        size_t post_data_content_length=0;
        size_t remaining_post_data_content_length=0;
        
        if( request.get_request_type() == request.REQUEST_POST )
        {
          jdk_log_debug1( "request is POST" );
          
          // extract additional data from tmp_buf and read the rest of the post data into request_buf
          const jdk_http_request_header::value_t *v;
          if( request.find_entry("content-length:", &v )!=-1 )
          {						
            post_data_content_length = v->strtol(10);
            jdk_log_debug1( "Content length is %d", post_data_content_length );	
            if( post_data_content_length>0 )
            {    
              remaining_post_data_content_length = post_data_content_length;
            }
            else
            {
              remaining_post_data_content_length = 0;
              post_data_content_length = 0;							    
            }
          }
          else
          {
            post_data_content_length = 0;
            remaining_post_data_content_length = 0;
          }
          
          // yup, we may still have data in buf[] that needs to be sent as well.
          size_t pos = request.get_buf_pos();
          size_t len = tmp_buf_len - pos;
          if( post_data_content_length>0 )
          {
            request_buf.resize( post_data_content_length );
            
            if( len>0 )
            {						
              jdk_log_debug1( "appending %d bytes of pre-buffer at %d", len, pos );
              request_buf.append_from_data(
                &tmp_buf[pos],
                len
                );
            }
            
            remaining_post_data_content_length = post_data_content_length - len;
            jdk_log_debug1( "remaining post data is %d, about to read it.", remaining_post_data_content_length );
            
            if( remaining_post_data_content_length>0 )
            {
              size_t additional_len = sock.read_data( request_buf.get_data()+len, remaining_post_data_content_length );
              if( additional_len!=remaining_post_data_content_length )
                return false;
            }
            request_buf.set_data_length( post_data_content_length );
          }
          else
          {
            //jdk_log_debug3( "(%d) %s no pre-sending of POST data", my_id, incoming_addr.c_str()  );
          }
          
        }
        
        
        return request.is_valid();
      }
    }
  }
  
  return false;
}

bool jdk_http_server_worker::send_response(jdk_client_socket &sock)
{
  jdk_str<64> len;
  len.form("%ld", response_buf.get_data_length() );
  response.add_entry( "Content-Length:", len );
  response.add_entry( "Connection: ", "close" );
  
  
  if( response.flatten( response_flattened) )
  {
    //jdk_log(JDK_LOG_DEBUG1,"Header len is %d", response_flattened.get_data_length() );
    //response_flattened.append_from_buf( response_buf );
    
    size_t wrlen = sock.write_data_block( 
      response_flattened.get_data(),
      response_flattened.get_data_length()
      );
    
    wrlen += sock.write_data_block( 
      response_buf.get_data(),
      response_buf.get_data_length()
      );
    
    if( wrlen==response_flattened.get_data_length()+response_buf.get_data_length())
    {
      jdk_log( JDK_LOG_DEBUG4, "Wrote proper data length to socket" );
      sock.close();
      return true;
    }
    else
    {
      jdk_log( JDK_LOG_DEBUG3, "Wrote %d bytes, wanted to write %d bytes", wrlen, response_flattened.get_data_length() );
    }
  }
  sock.close();
  return false;
}

bool jdk_http_server_worker::send_error( jdk_client_socket &sock, int code )
{
  jdk_log( JDK_LOG_INFO, "Error code sent to client: %d", code );							
  jdk_sprintf( tmp_buf, 
           "HTTP/1.0 %d Error\r\nConnection: close\r\nContent-type: text/html;charset=UTF-8\r\n\r\n"
           "<html><body><h1>Error 404</h1><p>Item not found</p></body></html>", code );
  if( sock.write_string_block( tmp_buf ) )
    return true;
  return false;
}


jdk_http_generator_bindir::jdk_http_generator_bindir( 
  const char *group_, 
  const char *prefix_
  ) : group( group_ ), prefix(prefix_)
{
}

// handle_request returns true if it knows how to handle this request
// returns false if it should be given to a different generator
bool jdk_http_generator_bindir::handle_request(
  const jdk_http_request_header &request,
  const jdk_dynbuf &request_data,
  jdk_http_response_header &response,
  jdk_dynbuf &response_data,
  const jdk_string &connection_id
  )
{
  const char *fname;
  
  if( request.get_url().path.ncmp( prefix, prefix.len() )==0 )
    fname = request.get_url().path.c_str()+prefix.len();
  else
    return false;
  
  jdk_bindir *d = jdk_bindir_find( group.c_str(), fname );
  if( d )
  {
    response.clear();
    response_data.clear();
    
    const char *ext = strrchr(fname,'.' );
    const char *mime="application/octet-stream";
    
    if( ext )
    {
      ++ext;
      if( strcmp( ext, "jpg")==0 )
        mime="image/jpeg";
      else if( strcmp( ext, "gif")==0 )
        mime="image/gif";
      else if( strcmp( ext, "html")==0 )
        mime="text/html; charset=UTF-8";
      else if( strcmp( ext, "txt")==0 )
        mime="text/plain; charset=UTF-8";
    }
    
    response.set_http_version( jdk_str<64>("HTTP/1.0") );
    response.set_http_response_code( 200 );
    response.set_http_response_text( jdk_str<64>("OK") );
    response.add_entry(	"Content-type:", mime);
    jdk_str<64> len; len.form( "%d", d->length );
    //response.add_entry( "Content-length:", len.c_str() );
    //response.add_entry( "Connection:", "close" );
    response.set_valid();	
    response_data.append_from_data( d->data, d->length );
    jdk_log( JDK_LOG_DEBUG1, "bindir file served: %s", fname );
    return true;
  }
  else
  {
    return false;
  }
}

