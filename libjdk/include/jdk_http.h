#ifndef _JDK_HTTP_H
#define _JDK_HTTP_H

#include "jdk_dynbuf.h"
#include "jdk_linesplit.h"
#include "jdk_url.h"
#include "jdk_map.h"
#include "jdk_socket.h"

class jdk_http_request_header
{
public:
  static const char *http_methods[];
  
  typedef jdk_str<256> key_t;
  typedef jdk_str<8192> value_t;
  typedef jdk_pair< key_t, value_t > pair_t;
  
  enum request_t
    {
      REQUEST_UNKNOWN=0,
      REQUEST_ACL,
      REQUEST_BASELINE_CONTROL,
      REQUEST_BIND,
      REQUEST_CHECKIN,
      REQUEST_CHECKOUT,
      REQUEST_CONNECT,
      REQUEST_COPY,
      REQUEST_DELETE,
      REQUEST_GET,
      REQUEST_HEAD,
      REQUEST_LABEL,
      REQUEST_LINK,
      REQUEST_LOCK,
      REQUEST_MERGE,
      REQUEST_MKACTIVITY,
      REQUEST_MKCOL,
      REQUEST_MKREDIRECTREF,
      REQUEST_MKWORKSPACE,
      REQUEST_MOVE,
      REQUEST_OPTIONS,
      REQUEST_ORDERPATCH,
      REQUEST_PATCH,
      REQUEST_POST,
      REQUEST_PROPFIND,
      REQUEST_PROPPATCH,
      REQUEST_PUT,
      REQUEST_REBIND,
      REQUEST_REPORT,
      REQUEST_SEARCH,
      REQUEST_TRACE,
      REQUEST_UNBIND,
      REQUEST_UNCHECKOUT,
      REQUEST_UNLINK,
      REQUEST_UNLOCK,
      REQUEST_UPDATE,
      REQUEST_UPDATEREDIRECTREF,
      REQUEST_VERSION_CONTROL,
      MAX_REQUEST
    };
  
  jdk_http_request_header() :
    map(),
    splitter(8192),
    valid(false),
    request_type( jdk_http_request_header::REQUEST_UNKNOWN ),
    line_count(0),
    url(),
    http_version()
    
    {
    }

  jdk_http_request_header( const jdk_http_request_header &o ) :
    map(o.map),
    splitter(o.splitter),
    valid(o.valid),
    request_type( o.request_type ),
    line_count(o.line_count),
    url(o.url),
    http_version(o.http_version)    
    {
    }
  
  
  void clear();
  
  bool parse_buffer( const char *buf, size_t len );
  
  bool parse_buffer( const jdk_dynbuf &buf )
    {
      return parse_buffer( (const char *)buf.get_data(), buf.get_data_length() );
    }
  
  bool is_valid() const
    {
      return valid;
    }	

  bool send_to_socket( jdk_client_socket &sock, bool full_url=false )
    {
      bool r=false;
      jdk_dynbuf buf;
      flatten(buf, full_url);
      if( sock.write_data_block(buf) == (int)buf.get_data_length() )
      {
        r=true;
      }
      return r;
    }
  
  bool parse_socket( 
    jdk_client_socket &sock, 
    jdk_dynbuf &request_additional_data
    )
    {
      bool r = false;
      char buf[8192];
      size_t len=0;
      
      request_additional_data.clear();
      
      while(1)
      {
        len = sock.read_data( buf, sizeof(buf), 30 );
        if( len<=0 )
          break;
        if( parse_buffer( buf, len ) )
        {
          // parsed it all!
          // if there is any data left over from parsing, stick it in the data_from_client queue
          size_t data_pos =  get_buf_pos();
          if( data_pos<len )
          {
            request_additional_data.append_from_data( &buf[data_pos], len-data_pos );
            clear_buf_pos();
          }
          
          // and return success
          r=true;
          break;
        }
      }        
      return r;
    }
  
	
	void set_valid( bool f=true )
    {
	    valid = f;
    }
  
  size_t get_buf_pos() const
    {
      return splitter.get_buf_pos();
    }
	
	void clear_buf_pos()
    {
      splitter.clear();	
    }
	
  
  bool flatten( jdk_dynbuf &dest, bool full_url ) const;
  
	const jdk_url &get_url() const
    {
	    return url;
    }
	
	void set_url( const jdk_url &url_ )
    {
	    url = url_;
    }
	
	request_t get_request_type() const
    {
	    return request_type;
    }
	
	void set_request_type( jdk_http_request_header::request_t t )
    {
	    request_type = t;
    }
	
	bool has_entry( const key_t &k ) const;			
	int find_entry( const key_t &k, const value_t **v, int start_pos=0 ) const;
	void delete_entry( const key_t &k );
	bool set_entry( const key_t &k, const value_t &v );
  bool add_entry( const key_t &k, const value_t &v );
  
	value_t get( const key_t &k ) const
    {
      int i=find( k );
      if( i>=0 )
      {
        return map.getvalue(i);
      }
      else
      {
        return "";
      }
    }
	
	size_t count() const
    {
      return map.count();
    }		
	
	int find( const key_t &k, size_t start_pos=0 ) const
    {
      return map.find( k, start_pos );
    }
	
  bool set( const pair_t &s )
    {
      return map.set( s.key, s.value );
    }
  
  bool set( const key_t &k, const value_t &v )
    {
      return map.set( k, v );
    }
  
  void remove( int i )
    {
      map.remove(i);
    }		
  
	const pair_t * get( int i ) const
    {
	    return map.get(i);
    }
	
  const jdk_string &get_http_version() const
    {
      return http_version;
    }
  
  void set_http_version( const jdk_string &v )
    {
      http_version=v;
    }
  
  jdk_map<key_t,value_t> map;
  
private:		
  
  bool parse_first_line( const jdk_string &line );
  
  jdk_linesplit splitter;    
	bool valid;    
	request_t request_type;	
  jdk_str<128> request_type_extended;
	int line_count;
	jdk_url url;
  jdk_str<64> http_version;
};



class jdk_http_response_header
{
public:
  
  typedef jdk_str<256> key_t;
  typedef jdk_str<8192> value_t;
  typedef jdk_pair< key_t, value_t > pair_t;
  
  jdk_http_response_header() :
    map(),
    splitter(8192),
    valid(false),
    line_count(0),
    http_version(""),
    http_response_code(0),
    http_response_text("")
    {
    }    
  
  void clear();
  
  bool parse_buffer( const char *buf, size_t len );
  
  bool parse_buffer( const jdk_dynbuf &buf )
    {
      return parse_buffer( (const char *)buf.get_data(), buf.get_data_length() );
    }
  
  bool is_valid() const
    {
      return valid;
    }	

  bool send_to_socket( jdk_client_socket &sock )
    {
      bool r=false;
      jdk_dynbuf buf;
      flatten(buf);
      if( sock.write_data_block(buf) == (int)buf.get_data_length() )
      {
        r=true;
      }
      return r;
    }
  
  bool parse_socket( 
    jdk_client_socket &sock, 
    jdk_dynbuf &response_additional_data
    )
    {
      bool r = false;
      char buf[8192];
      size_t len=0;
      
      response_additional_data.clear();
      
      while(1)
      {
        len = sock.read_data( buf, sizeof(buf), 30 );
        if( len<=0 )
          break;
        if( parse_buffer( buf, len ) )
        {
          // parsed it all!
          // if there is any data left over from parsing, stick it in the data_from_client queue
          size_t data_pos =  get_buf_pos();
          if( data_pos<len )
          {
            response_additional_data.append_from_data( &buf[data_pos], len-data_pos );
            clear_buf_pos();
          }
          
          // and return success
          r=true;
          break;
        }
      }        
      return r;
    }
	
	void set_valid(bool f=true)
    {
	    valid=f;
    }
  
  size_t get_buf_pos() const
    {
      return splitter.get_buf_pos();
    }

	void clear_buf_pos()
    {
      splitter.clear();	
    }
  
  bool flatten( jdk_dynbuf &dest ) const;
  
  
	const jdk_str<64> &get_http_version() const
    {
	    return http_version;
    }
	
	int get_http_response_code() const
    {
	    return http_response_code;
    }
	
	const jdk_str<4096> &get_http_response_text() const
    {
	    return http_response_text;
    }
	
	
	void set_http_version( const jdk_string &v )
    {
	    http_version = v;
    }
	
	void set_http_response_code( int c )
    {
	    http_response_code = c;
    }
	
	void set_http_response_text( const jdk_string &v )
    {
	    http_response_text = v;
    }
	
	bool has_entry( const key_t &k ) const;		
	int find_entry( const key_t &k, const value_t **v, int start_pos=0 ) const;
	void delete_entry( const key_t &k );
	bool set_entry( const key_t &k, const value_t &v );
  bool add_entry( const key_t &k, const value_t &v );	
	bool add_entry( const char *k, const char *v );
  
	value_t get( const key_t &k ) const
    {
      int i=find( k );
      if( i>=0 )
			{
        return map.getvalue(i);
			}
      else
			{
        return "";
			}
    }
  
	size_t count() const
    {
      return map.count();
    }		
	
	int find( const key_t &k, size_t start_pos=0 ) const
    {
      return map.find( k, start_pos );
    }
	
  bool set( const pair_t &s )
    {
      return map.set( s.key, s.value );
    }
  
  bool set( const key_t &k, const value_t &v )
    {
      return map.set( k, v );
    }
  
  void remove( int i )
    {
      map.remove(i);
    }		
  
	const pair_t * get( int i ) const
    {
	    return map.get(i);
    }
  
  
  
  jdk_map<key_t,value_t> map;
private:		
  
  bool parse_first_line( const jdk_string &line );
  
  jdk_linesplit splitter;    
	bool valid;    
	int line_count;
	
	jdk_str<64> http_version;
	int http_response_code;
	jdk_str<4096> http_response_text;
	
};




#endif
