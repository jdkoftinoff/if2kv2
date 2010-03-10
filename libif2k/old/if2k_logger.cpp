#include "jdk_world.h"
#include "if2k_logger.h"

char *if2_logger::access_types[16] =
{
  "Unknown URL",
  "Good URL",
  "Bad URL",
  "Bad Phrase"
  "Censored Phrase",
  "Good Newsgroup",
  "Bad Newsgroup",
  "Bad email content",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  ""    
};


if2_multi_logger::if2_multi_logger( 
  const jdk_settings &settings
  ) : 
  num_loggers(0), 
  max_loggers( settings.get_long("url_loggers") ),
  logger(),
  logger_mutex()
{		
  logger = new if2_logger * [max_loggers];
  char logger_name[32] = "logger#";
  for( int i=0; i<max_loggers; ++i )
  {
    logger_name[6] = i+'A';
    
    jdk_str<64> type = settings.get_prefixed( logger_name, "_type" );
    
    if( type.icmp("none")==0 )
    {
      continue;	
    }
    if( type.icmp("file")==0 )
    {
      add_logger( new if2_logger_file( settings, logger_name ) );
      continue;
    }
    if( type.icmp("cgi")==0 )
    {
      add_logger( new if2_logger_cgi( settings, logger_name ) );
      continue;
    }
    if( type.icmp("socket")==0 )
    {
      add_logger( new if2_logger_socket( settings, logger_name ) );
      continue;
    }
    if( type.icmp("mail")==0 )
    {
      add_logger( new if2_logger_mail( settings, logger_name ) );
      continue;
    }
    
    jdk_log( JDK_LOG_ERROR, "unknown url logger type: %s", type.c_str() );
  }
  
}

if2_multi_logger::~if2_multi_logger()
{
  for( int i=0; i<num_loggers; ++i )
  {
    delete logger[i];	
  }
  delete [] logger;
}

bool if2_multi_logger::add_logger( if2_logger *l )
{
  if( num_loggers<max_loggers )
  {
    logger[num_loggers++] = l;
    return true;
  }
  else
  {
    delete l;
    JDK_THROW_ERROR( "Too many loggers added to list", "" );
    return false;
  }
  
}


if2_logger *if2_multi_logger::get_logger(int i)
{
  return logger[i];	
}


void if2_multi_logger::log( 
  const if2_kernel_result &type,					 
  const char *client_address,
  const char *url,
  const char *block_pattern,
  bool override
  )
{
  jdk_synchronized(logger_mutex);
  for( int i=0; i<num_loggers; ++i )
  {
    logger[i]->log( type, client_address, url, block_pattern, override );	
  }		
}




if2_logger::if2_logger( const jdk_settings &settings, const jdk_settings::key_t &prefix )
  : 
  ignore_images( settings, prefix.c_str(), "_ignore_images" ),
  enable_override_log( settings, prefix.c_str(), "_log_override" ),
  enable_all( settings, prefix.c_str(), "_enable" )
{
  jdk_settings::key_t level_prefix = prefix;
  level_prefix.cat( "_level_" );
  
  for( int i=0; i<9; ++i )
  {
    jdk_str<32> num;			
    num.form("%d", i );
    enable_map[i] = new jdk_live_long( settings, level_prefix, num );
  }
}

if2_logger::~if2_logger()
{
  for( int i=0; i<9; ++i )
  {
    delete enable_map[i];
  }
}


bool if2_logger::validate_type(const if2_kernel_result &type,const char *url, bool override )
{
  if( enable_all && (enable_map[type.key/100]->get() || (enable_override_log && override)))
  {
    if( ignore_images )
    {
      const char *extension = jdk_find_extension( url );
      if( extension )
      {
        if( jdk_stricmp( extension, ".gif" )==0
            || jdk_stricmp( extension, ".jpg" )==0
            || jdk_stricmp( extension, ".jpeg" )==0
            || jdk_stricmp( extension, ".png" )==0 )
        {
          return false;	
        }					
      }				
    }
    return true;
  }
  return false;		
}

void if2_logger::format( 
  jdk_str<8192> &result, 
  const if2_kernel_result &type, 
  const char *client_address,
  const char *url,
  const char *block_pattern,
  bool override
  )
{
  if( block_pattern==0 )
    block_pattern="";
  //jdk_str<8192> logline;
  jdk_str<1024> date;
  
  time_t timep;
  time(&timep);
#if !JDK_IS_NETBSD	   
  strftime( date.c_str(), date.getmaxlen(), "%a, %d %b %Y %H:%M:%S", localtime(&timep) );
#endif		
  
  jdk_str<256> client_name;
  convert_ip_to_name( client_address, client_name.c_str(), client_name.getmaxlen() );
  
  if( type.value==true )
  {
    result.form("%s\tACCESS %03d %s\t%s\t%s\t%s", date.c_str(), type.key, access_types[type.key/100], client_name.c_str(), url, block_pattern ? block_pattern : "" );
  }
  else
  {
    result.form("%s\t%sBLOCK %03d %s\t%s\t%s\t%s", date.c_str(), override ? "OVERRIDE " : "", type.key, access_types[type.key/100], client_name.c_str(), url, block_pattern ? block_pattern : "" );
  }
}

void if2_logger::convert_ip_to_name( const char *client_address, char *client_name, int client_name_len )
{
  if( jdk_strcmp(client_address,"127.0.0.1")==0 )
  {
    if( gethostname( client_name, client_name_len )<0 )
    {
      jdk_strncpy( client_name, client_address, client_name_len );
    }
  }
  else
  {
    jdk_gethostbyaddr( client_address, client_name, client_name_len );
  }
}



if2_logger_file::if2_logger_file( 
  const jdk_settings &settings, 
  const jdk_settings::key_t setting_prefix 
  ) 
  : 
  if2_logger( settings, setting_prefix ),
  orig_fname( settings, setting_prefix.c_str(), "_file" )
{
}

void if2_logger_file::log(
  const if2_kernel_result &type,
  const char *client_address,
  const char *url,
  const char *block_pattern,
  bool override
  )
{
  if( validate_type(type,url,override) )
  {
    jdk_string_filename fname;
    jdk_process_path( orig_fname.get().c_str(), fname.c_str(), fname.getmaxlen() );
    
    int fd = open(fname.c_str(),O_CREAT | O_APPEND | O_WRONLY, 0640 );
    if( fd>=0 )
    {
      jdk_str<8192> result;
      format( result, type, client_address, url, block_pattern, override );
      result.cat("\n");
      int len = result.len();
      if( write( fd, result.c_str(), len )!=len )
      {
        jdk_log( JDK_LOG_ERROR, "Error writing to log file '%s'", fname.c_str() );
      }
      
      close(fd);
    }
    else
    {
      jdk_log( JDK_LOG_ERROR, "Error opening log file '%s'", fname.c_str() );					
    }
    
  }
}


if2_logger_cgi::if2_logger_cgi( 
  const jdk_settings &settings,
  const jdk_settings::key_t setting_prefix
  ) 
  : 
  if2_logger( settings, setting_prefix ),
  dest_url( settings, setting_prefix.c_str(), "_url" ),
  proxy( settings, setting_prefix.c_str(), "_proxy" ),
  parent_server_url( settings, "parent_server_url" )
{
}

void if2_logger_cgi::log(
  const if2_kernel_result &type,					 
  const char *client_address,
  const char *url,
  const char *block_pattern,
  bool override
  )
{
  if( validate_type(type,url,override) )
  {
    jdk_str<8192> result;
    format( result, type, client_address, url, block_pattern, override );
    jdk_str<8192> escaped;
    jdk_cgi_escape( result, escaped );
    jdk_dynbuf dummy_response;
    jdk_str<8192> full_url;
    full_url.cat(parent_server_url.get());
    full_url.cat(dest_url.get());
    full_url.cat(escaped);
    jdk_http_response_header response_header;
    if( jdk_http_get( full_url.c_str(), &dummy_response, 8192, &response_header, proxy.get().c_str(), false )!=200 )
    {
      jdk_log( JDK_LOG_ERROR, "Error writing to log url '%s'", dest_url.get().c_str() );
    }
    
  }
}


if2_logger_socket::if2_logger_socket( 
  const jdk_settings &settings,
  const jdk_settings::key_t setting_prefix
  ) 
  : 
  if2_logger( settings, setting_prefix ),
  host( settings, setting_prefix.c_str(), "_host" )
  
{
}

void if2_logger_socket::log(
  const if2_kernel_result &type,
  const char *client_address,
  const char *url,
  const char *block_pattern,
  bool override
  )
{
  if( validate_type(type,url,override) )
  {
    jdk_str<8192> result;
    format( result, type, client_address, url, block_pattern, override );
    result.cat("\r\n");
    jdk_inet_client_socket sock;
    if( sock.make_connection( host.get(),0,false ) )
    {
      if( !sock.write_string_block( result ) )
      {
        jdk_log( JDK_LOG_ERROR, "Error writing to opened socket '%s'", host.get().c_str() );
      }
      sock.close();
    }
    else
    {
      jdk_log( JDK_LOG_ERROR, "Error connecting to '%s'", host.get().c_str() );
    }
  }
}


if2_logger_mail_thread::if2_logger_mail_thread(
  const jdk_settings &settings,
  const jdk_string &prefix
  )
  : 		
  smtp_server( settings, prefix.c_str(), "_smtp_server" ),
  mail_from( settings, prefix.c_str(), "_mail_from" ),
  mail_to( settings, prefix.c_str(), "_mail_to" ),
  mail_subject( settings, prefix.c_str(), "_mail_subject" ),
  mail_helo( settings, prefix.c_str(), "_smtp_helo" ),
//	minutes_per_email( settings, prefix.c_str(), "_mail_minutes" ),
  body(),
  body_mutex()
{
}

if2_logger_mail_thread::~if2_logger_mail_thread()
{
}

void if2_logger_mail_thread::append_line( const jdk_string &s )
{
  jdk_synchronized(body_mutex);
  body.append_from_string(s);
}

void if2_logger_mail_thread::main()
{
  time_t last_email_time=time(0);
  
  
  while( !please_stop )
  {
    int32 minutes;
    
    do
    {
      if( please_stop )
        return;
      sleep(1);
      
      minutes  = 5; //minutes_per_email.get();
      if( minutes<1 )
        minutes=1;
      if( minutes>60*24*7 )
        minutes=60*24*7;
      
    } while( (time(0)-last_email_time) < minutes * 60 );
    
    last_email_time=time(0);
    
    {
      jdk_synchronized(body_mutex);
      if( body.get_data_length()>0 )
      {
        if( jdk_smtp_send( mail_helo.get(), smtp_server.get(), mail_from.get(), mail_to.get(), mail_subject.get(), body ) )
        {
          body.clear();
        }
        jdk_log( JDK_LOG_INFO, "Sent email to %s via smtp:%s", mail_to.get().c_str(), smtp_server.get().c_str() );				   
      }
    }
  }
}


if2_logger_mail::if2_logger_mail(
  const jdk_settings &settings,
  const jdk_settings::key_t setting_prefix
  ) 
  : 
  if2_logger( settings, setting_prefix.c_str() ),
  mail_thread( settings, setting_prefix )			   
{
  mail_thread.run();
}


if2_logger_mail::~if2_logger_mail()
{
  mail_thread.thread_pleasestop();
  jdk_sleep(2);
}

void if2_logger_mail::log(
  const if2_kernel_result &type,					 
  const char *client_address,
  const char *url,
  const char *block_pattern,
  bool override
  )
{
  if( validate_type(type,url,override) )
  {
    jdk_str<8192> result;
    format( result, type, client_address, url, block_pattern, override );
    result.cat("\r\n");
    
    mail_thread.append_line( result );
  }
}

void if2_logger_mail::format( 
  jdk_str<8192> &result, 
  const if2_kernel_result &type,						
  const char *client_address,
  const char *url,
  const char *block_pattern,
  bool override				
  )
{
  if( block_pattern==0 )
    block_pattern="";
  //jdk_str<8192> logline;
  jdk_str<1024> date;
  
  time_t timep;
  time(&timep);
#if !JDK_IS_NETBSD	   
  strftime( date.c_str(), date.getmaxlen(), "%a, %d %b %Y %H:%M:%S", localtime(&timep) );
#endif		
  
  jdk_str<256> client_name;
  convert_ip_to_name( client_address, client_name.c_str(), client_name.getmaxlen() );
  
  if( type.value==true )
  {
    result.form("Date:\t%s\r\nClient:\t%s\r\nURL ACCESS: (%03d)\t%s\r\nMatch:\t%s\t%s\r\n", 
                date.c_str(), 
                client_name.c_str(), 
                type.key, 
                url, 
                access_types[ type.key/100 ],
                block_pattern ? block_pattern : "" 
      );
  }
  else
  {
    result.form("Date:\t%s\r\nClient:\t%s\r\nURL %sBLOCK (%03d):\t%s\r\nMatch:\t%s\t%s\r\n", 
                date.c_str(), 
                client_name.c_str(), 
                override ? "OVERRIDE " : "", 
                type.key, 
                url, 
                access_types[ type.key/100 ],
                block_pattern ? block_pattern : "" 
      );			
  }
}


