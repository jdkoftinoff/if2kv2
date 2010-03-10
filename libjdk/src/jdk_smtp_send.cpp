#include "jdk_world.h"
#include "jdk_socket.h"
#include "jdk_smtp_send.h"

#define SMTP_SEND_TIMEOUT (30)


#ifndef JDK_IS_NETBSD
#define JDK_IS_NETBSD 0
#endif

#if JDK_IS_NETBSD

extern "C" size_t strftime(char *buf, size_t maxsize, const char *format,
	                   const struct tm *timeptr);

#endif

static int jdk_smtp_get_response( jdk_inet_client_socket &s )
{
  jdk_str<4096> response;
  do
  {
    size_t len =s.read_string_with_timeout( response, SMTP_SEND_TIMEOUT );
    if( len<4 )
    {
      return -1;
    }
  } while( response.get(3)=='-' );
  
  return atoi(response.c_str());
}

static bool jdk_smtp_send_buf(
  jdk_inet_client_socket &s,
  const jdk_buf &buf
  )
{
  jdk_str<1024> line;
  int pos=0;
  
  while( (pos=buf.extract_to_string(line,pos))>=0 )
  {
    line.cat("\r\n");
    if( line.get(0)=='.' )
    {
      if( !s.write_string_block( "." ) )
      {
        return false;	
      }
      
    }		
    if( !s.write_string_block( line ) )
    {
      return false;	
    }
    
  }
  return true;
}


static bool jdk_smtp_send_line( 
  jdk_inet_client_socket &s,
  const char *line
  )
{
  s.write_string_block( line );
  int ret = jdk_smtp_get_response( s );
  return ret>=0 && ret<400;
}

static bool jdk_smtp_send_rcpt(
  jdk_inet_client_socket &s,
  const jdk_array< jdk_str<512> > &recipients
  )
{
  bool r=false;
  for( size_t i=0; i<recipients.count(); ++i )
  {
    const jdk_str<512> *cur_rcpt = recipients.get(i);
    if( cur_rcpt )
    {
      jdk_str<4096> rcpt_to;
      rcpt_to.form("RCPT TO:%s\r\n", cur_rcpt->c_str() );
      
      // return true if any of recipients is ok.
      if( jdk_smtp_send_line( s, rcpt_to.c_str() )==true )
        r=true;
    }
  }
  
  return r;
}


bool jdk_smtp_send( 
  const jdk_string &smtp_helo,
  const jdk_string &smtp_server,
  const jdk_string &from,
  const jdk_string &to,
  const jdk_string &subject,
  const jdk_buf &body
  )
{
  jdk_inet_client_socket s;
  jdk_array< jdk_str<512> > recipients;
  
  // extract all comma delimited sections into recipients array
  {
    bool in_quotes=false;
    size_t len=to.len();
    jdk_str<512> *cur_recip = new jdk_str<512>;
    cur_recip->cat( '<' );
    for( size_t i=0; i<len; ++i )
    {
      char c=to.get(i);
      
      if( in_quotes )
      {
        if( c=='"' )
          in_quotes=false;
        continue;
      }
      if( c=='"' )
      {
        in_quotes=true;
        continue;
      }
      
      if( (c==',' || c==';' || jdk_isspace(c)) && cur_recip->len()>3 )
      {
        cur_recip->cat( '>' );
        recipients.add( cur_recip );
        cur_recip=new jdk_str<512>;
        cur_recip->cat( '<' );
      }
      if( c=='<' || c=='>' ) 
        continue;
      
      if( !jdk_isspace( c ) )
      {
        cur_recip->cat( c );
      }
    }
    if( cur_recip->len()>3 )
    {
      cur_recip->cat( '>' );	
      recipients.add( cur_recip );
    }
  }
  
  if( s.make_connection( smtp_server.c_str(), 25, 0, false ) )
  {
    jdk_str<4096> mail_from;
    jdk_str<4096> helo;
    helo.form( "HELO %s\r\n", smtp_helo.c_str() );
    mail_from.form( "MAIL FROM:%s\r\n", from.c_str() );
    jdk_dynbuf header;
    jdk_str<4096> mail_date;
    
#if 0
    SYSTEMTIME timep;
    GetLocalTime(&timep);
    struct tm t;
    t.tm_sec = timep.wSecond;
    t.tm_min = timep.wMinute;
    t.tm_hour = timep.wHour;
    t.tm_mday = timep.wDay;
    t.tm_mon = timep.wMonth;
    t.tm_year = timep.wYear;
    t.tm_wday = timep.wDayOfWeek;
    t.tm_yday = 0;
    t.tm_isdst = 0;
    
    strftime( mail_date.c_str(), 4094, "%a, %d %b %Y %H:%M:%S", &t );
#else
    time_t timep;
    timep = time(0);
    
    strftime( mail_date.c_str(), 4094, "%a, %d %b %Y %H:%M:%S %z (%Z)", jdk_localtime(&timep) );
#endif
    header.append_form( "Date: %s\r\n", mail_date.c_str() );
    header.append_form( "From: %s\r\n", from.c_str() );
    header.append_form( "To: %s\r\n", to.c_str() );
    header.append_form( "Subject: %s\r\n\r\n", subject.c_str() );		
    
    return jdk_smtp_get_response(s)
      && jdk_smtp_send_line( s, helo.c_str() )
      && jdk_smtp_send_line( s, mail_from.c_str() )
      && jdk_smtp_send_rcpt( s, recipients )
      && jdk_smtp_send_line( s, "DATA\r\n" )
      && jdk_smtp_send_buf( s, header )
      && jdk_smtp_send_buf( s, body )
      && jdk_smtp_send_line( s, "\r\n.\r\n" );				
  }
  return false;
}


