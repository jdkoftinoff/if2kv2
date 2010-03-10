#ifndef _JDK_REMOTE_BUF_H
#define _JDK_REMOTE_BUF_H

#include "jdk_string.h"
#include "jdk_dynbuf.h"
#include "jdk_util.h"
#include "jdk_settings.h"



class jdk_remote_buf
{
public:
#if 0
  jdk_remote_buf( 
    const jdk_settings &settings,
    const jdk_str<256> setting_field_prefix,
    long max_len,
    bool ignore_remote_if_local_exists 
    );
#endif

#if 0
  jdk_remote_buf(
    const jdk_string_filename &local_file_,
    const jdk_string_url &remote_url_,
    const jdk_string_url &proxy_,
    long max_len_,
    bool ignore_remote_if_local_exists
    );
#endif

  jdk_remote_buf(
    const jdk_string_filename &local_file_,
    const jdk_string_url &remote_url_,
    const jdk_string_url &proxy_,
    long max_len_,
    bool ignore_remote_if_local_exists,
    const jdk_string_url &proxy_username,
    const jdk_string_url &proxy_password,
    bool mark_as_hidden
    );
  
  void clear_buf();    // without erasing timestamp
  
	bool load_local();
  bool check_and_grab();
  bool check();
  bool grab();
  
  jdk_dynbuf &get_buf()
    {
      return buf;
    }
  
  const jdk_dynbuf &get_buf() const
    {
      return buf;
    }

  static void enable_remote_loading() { remote_enabled=true; }
  static void disable_remote_loading() { remote_enabled=true; }
  
protected:
  void mark_file_hidden( const jdk_string_filename &fname );
  void mark_file_unhidden( const jdk_string_filename &fname );
  jdk_string_url remote_url;
  jdk_string_filename local_file;
  jdk_string_filename local_timestamp_file;
  jdk_string_url proxy;
  jdk_string_url proxy_username;
  jdk_string_url proxy_password;
  bool mark_as_hidden;
  long max_len;
	bool remote_is_compressed;
	bool local_is_compressed;
  
  jdk_dynbuf buf;
  jdk_dynbuf timestamp;
	bool ignore_remote_if_local_exists;
	bool local_exists;	
  static volatile bool remote_enabled;
};



#endif
