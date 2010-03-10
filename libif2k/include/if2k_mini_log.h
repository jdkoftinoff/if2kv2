#ifndef __IF2K_MINI_LOG_H
#define __IF2K_MINI_LOG_H

#include "if2k_mini_config.h"
#include "if2k_mini_kernel.h"

#include "jdk_string.h"
#include "jdk_error.h"
#include "jdk_httprequest.h"
#include "jdk_cgi.h"
#include "jdk_settings.h"
#include "jdk_cgisettings.h"
#include "jdk_smtp_send.h"
#include "jdk_thread.h"
#include "jdk_util.h"


class if2k_mini_log_file_thread : public jdk_thread
{
public:
  if2k_mini_log_file_thread( jdk_settings &settings );
  
  ~if2k_mini_log_file_thread();
   
  void append_line( const jdk_string &s );
   
protected:
   void main();

   jdk_string_filename dest_file;
   int minutes_per_write;
	   
   jdk_dynbuf body;
   jdk_recursivemutex body_mutex;
};


class if2k_mini_log_mail_thread : public jdk_thread
{
public:
  if2k_mini_log_mail_thread( jdk_settings &settings );
  
  ~if2k_mini_log_mail_thread();
   
  void append_line( const jdk_string &s );
   
protected:
   void main();

   jdk_string_url smtp_server;
   jdk_string_url mail_from;
   jdk_string_url mail_to;
   jdk_string_url mail_subject;
   jdk_string_url mail_helo;
   long minutes_per_email;
	   
   jdk_dynbuf body;
   jdk_recursivemutex body_mutex;
};



class if2k_mini_log_socket_thread : public jdk_thread
{
public:
  if2k_mini_log_socket_thread( jdk_settings &settings );
  
  ~if2k_mini_log_socket_thread();
   
  void append_line( const jdk_string &s );
   
protected:
   void main();

   jdk_str<512> socket_destination_host_and_port;
   long minutes_per_send;
	   
   jdk_dynbuf body;
   jdk_recursivemutex body_mutex;
};

class if2k_mini_log_cgi_thread : public jdk_thread
{
public:
  if2k_mini_log_cgi_thread( jdk_settings &settings );
  
  ~if2k_mini_log_cgi_thread();
   
  void append_line( const jdk_string &s );
   
protected:
   void main();

   jdk_string_url cgi_prefix;
   jdk_string_url http_proxy;
   long minutes_per_send;
	   
   jdk_dynbuf body;
   jdk_recursivemutex body_mutex;
};


class if2k_mini_log
{
  if2k_mini_log( const if2k_mini_log & );
  const if2k_mini_log & operator = ( const if2k_mini_log & );
public:
	if2k_mini_log( const jdk_settings &settings );
	
	~if2k_mini_log();

	void log( 
    const if2k_mini_kernel_request &request
    );
	
protected:
	void format( 
    jdk_str<8192> &result, 
    const if2k_mini_kernel_request &type
    );

	void format_for_email( 
    const if2k_mini_kernel_request &type
    );

	void format_for_cgi( 
    const if2k_mini_kernel_request &type
    );

	void format_for_file( 
    const if2k_mini_kernel_request &type
    );

	void format_for_socket( 
    const if2k_mini_kernel_request &type
    );
  
protected:
	bool log_enable;
  bool log_file_enable;
  jdk_string_filename log_file_local;
  bool log_email_enable;

  bool log_all;
  bool log_bad;
  bool log_unknown;
  bool log_overridden;
  bool log_good;
  bool log_images;

  if2k_mini_log_socket_thread socket_thread;
  if2k_mini_log_mail_thread mail_thread;
  if2k_mini_log_file_thread file_thread;
  if2k_mini_log_cgi_thread cgi_thread;
};


#endif
