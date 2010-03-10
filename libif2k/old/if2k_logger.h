#ifndef __IF2_LOGGER_H
#define __IF2_LOGGER_H

#include "jdk_string.h"
#include "jdk_error.h"
#include "jdk_httprequest.h"
#include "jdk_cgi.h"
#include "jdk_settings.h"
#include "jdk_smtp_send.h"
#include "jdk_thread.h"
#include "jdk_util.h"

#include "jdk_pair.h"

typedef jdk_pair<int,bool> if2_kernel_result;

// violation levels:
//   0 = unknown
//   1 = known good
//   2+ = known bad

class if2_logger
{
    if2_logger( const if2_logger & );
    const if2_logger & operator = ( const if2_logger & );
public:
	if2_logger( const jdk_settings &settings, const jdk_settings::key_t &prefix );
	
	virtual ~if2_logger();
	
	
	virtual void log( 
					 const if2_kernel_result &type,
					 const char *client_address,
					 const char *url,
					 const char *block_pattern,
					 bool override
					 ) = 0;
	
protected:
	bool validate_type(const if2_kernel_result &type,const char *url, bool override );
	
	virtual void format( 
				jdk_str<8192> &result, 
				const if2_kernel_result &type, 
				const char *client_address,
				const char *url,
				const char *block_pattern,
				bool override
				);
	
	void convert_ip_to_name( const char *client_address, char *client_name, int client_name_len );
		
protected:
	jdk_live_long ignore_images;
	jdk_live_long enable_override_log;	
	jdk_live_long enable_all;
	jdk_live_long *enable_map[16];	
	
	static char *access_types[16];
};




class if2_logger_file : public if2_logger
{
public:
	if2_logger_file( 
					const jdk_settings &settings, 
					const jdk_settings::key_t setting_prefix 
					);
	
	void log(
					 const if2_kernel_result &type,
					 const char *client_address,
					 const char *url,
					 const char *block_pattern,
					 bool override
					 );
	
private:
	jdk_live_string_filename orig_fname;
};


class if2_logger_cgi : public if2_logger
{
public:
	if2_logger_cgi( 
				   const jdk_settings &settings,
				   const jdk_settings::key_t setting_prefix
				   );
				   
	virtual void log(
					 const if2_kernel_result &type,					 
					 const char *client_address,
					 const char *url,
					 const char *block_pattern,
					 bool override
					 );
	
private:
	jdk_live_string_url dest_url;
	jdk_live_string_url proxy;
	jdk_live_string_url parent_server_url;
};



class if2_logger_socket : public if2_logger
{
public:
	if2_logger_socket( 
				   const jdk_settings &settings,
				   const jdk_settings::key_t setting_prefix
				   ) ;
				   	
	void log(
					 const if2_kernel_result &type,
					 const char *client_address,
					 const char *url,
					 const char *block_pattern,
					 bool override
					 );
	
private:
	jdk_live_string_url host;
};

class if2_logger_mail_thread : public jdk_thread
{
public:
   if2_logger_mail_thread(
                           const jdk_settings &settings,
			               const jdk_string &prefix
						   );
						   
   ~if2_logger_mail_thread();
   
   void append_line( const jdk_string &s );
   
protected:
   void main();

   jdk_live_string_url smtp_server;
   jdk_live_string_url mail_from;
   jdk_live_string_url mail_to;
   jdk_live_string_url mail_subject;
   jdk_live_string_url mail_helo;
   //jdk_live_long minutes_per_email;
	   
   jdk_dynbuf body;
   jdk_recursivemutex body_mutex;
};

class if2_logger_mail : public if2_logger
{
public:
	if2_logger_mail(
				   const jdk_settings &settings,
				   const jdk_settings::key_t setting_prefix
				   );

   ~if2_logger_mail();
   
	void log(
					 const if2_kernel_result &type,					 
					 const char *client_address,
					 const char *url,
					 const char *block_pattern,
					 bool override
					 );

protected:
	void format( 
				jdk_str<8192> &result, 
				const if2_kernel_result &type,						
				const char *client_address,
				const char *url,
				const char *block_pattern,
				bool override				
				);
private:
    if2_logger_mail_thread mail_thread;
};




class if2_multi_logger
{
    if2_multi_logger( const if2_multi_logger & );
    const if2_multi_logger & operator = ( if2_multi_logger & );
    
public:
	if2_multi_logger( 
					 const jdk_settings &settings
					 );
	
	virtual ~if2_multi_logger();
	
    bool add_logger( if2_logger *l );
	
	
	if2_logger *get_logger(int i);

	
	virtual void log( 
					 const if2_kernel_result &type,					 
					 const char *client_address,
					 const char *url,
					 const char *block_pattern,
					 bool override
					 );
	
private:
	int num_loggers;
	int max_loggers;
	if2_logger **logger;
	jdk_mutex logger_mutex;
};



#endif
