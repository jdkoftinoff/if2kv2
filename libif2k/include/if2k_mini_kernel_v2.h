#ifndef __IF2K_MINI_KERNEL_V2_H
#define __IF2K_MINI_KERNEL_V2_H

#include "if2k_mini_kernel.h"
#include "if2k_mini_scanner.h"
#include "jdk_serial_number.h"
#include "jdk_fork_server.h"

extern jdk_setting_description if2k_mini_kernel_defaults[];


#if JDK_IS_WIN32
#include "wininet.h"

#define EPOCHFILETIME (116444736000000000)

inline int gettimeofday(struct timeval *tv, void *tz)
{
   FILETIME        ft;
   LARGE_INTEGER   li;
   __int64         t;

   if (tv)
   {
      GetSystemTimeAsFileTime(&ft);
      li.LowPart  = ft.dwLowDateTime;
      li.HighPart = ft.dwHighDateTime;
      t  = li.QuadPart;       /* In 100-nanosecond  intervals */
      t -= EPOCHFILETIME;     /* Offset to the Epoch  time */
      t /= 10;                /* In microseconds */
      tv->tv_sec  = (long)(t / 1000000);
      tv->tv_usec = (long)(t % 1000000);
   }
   return 0;
}
#endif


inline bool if2kd_validate_license( jdk_settings &settings )
{
  bool is_demo=false;
  jdk_serial_number serial;
  serial.load( settings );

  if( serial.info.cmp("demo")==0 )
  {
    is_demo=true;
    if( serial.expires_code.is_clear() )
    {
      // fill in default expires and expires code for one month from now.
      struct timeval current_date;
      gettimeofday(&current_date,0);
      
      current_date.tv_sec += (30*(60*60*24)); // add 30 days worth of seconds
      serial.expires_code.form("%lu", (unsigned long) (current_date.tv_sec) );
    }
  }

  bool is_valid = serial.is_valid_code( jdk_string_static(IF2K_MINI_SERVER_PRODUCT_ID) );

  if( is_valid || is_demo )
  {
    is_valid=true;
    long expires_code = serial.expires_code.strtol();
    if( expires_code!=-1 )
    {
      struct timeval t;
      if( gettimeofday( &t, 0 )==0 )
      {
        if( expires_code < t.tv_sec )
        {
          is_valid=false;
        }
      }
      else
      {
        is_valid=false;
      }
    }
  }
  settings.set( "license.valid", is_valid ? "1": "0" );

  jdk_log( JDK_LOG_INFO, is_valid ? "License is valid" : "License is not valid");
  return is_valid;
}



class if2k_mini_kernel_v2_settings_collection : public jdk_fork_server_reloader
{
public:
  if2k_mini_kernel_v2_settings_collection( int argc, char **argv )
    : args_settings(0,argc,argv),
      default_settings(if2k_mini_kernel_defaults)
  {
    reload();
  }

  ~if2k_mini_kernel_v2_settings_collection()
  {
  }


  bool load_system_install()
  {
#if JDK_IS_WIN32
    return install_settings.load_registry( HKEY_LOCAL_MACHINE, IF2K_MINI_INSTALL_REGISTRY_LOCATION );
#else
    jdk_string_filename f( default_settings.get("kernel.settings.master.local") );
    jdk_log( JDK_LOG_DEBUG1, "Loading install settings from: %s", f.c_str() );
    bool r=install_settings.load_file(f);
    jdk_log( JDK_LOG_DEBUG1, "read=%d", (int)r );
    return r;
#endif
  }

  bool load_system_license()
  {
#if JDK_IS_WIN32
    return license_settings.load_registry( HKEY_LOCAL_MACHINE, IF2K_MINI_LICENSE_REGISTRY_LOCATION );
#else
    jdk_string_filename f( default_settings.get("kernel.settings.license.local") );
    jdk_log( JDK_LOG_DEBUG1, "Loading License from: %s", f.c_str() );
    bool r=license_settings.load_file(f);
    jdk_log( JDK_LOG_DEBUG1, "read=%d", (int)r );
    return r;
#endif
  }

  bool load_system_additional()
  {
#if JDK_IS_WIN32
    return additional_settings.load_registry( HKEY_LOCAL_MACHINE, IF2K_MINI_ADDITIONAL_REGISTRY_LOCATION );
#else
    jdk_string_filename f( default_settings.get("kernel.settings.additional.local") );
    jdk_log( JDK_LOG_DEBUG1, "Loading additional settings from: %s", f.c_str() );
    bool r=additional_settings.load_file(f);
    jdk_log( JDK_LOG_DEBUG1, "read=%d", (int)r );
    return r;
#endif
  }


  bool load_remote_additional()
  {
    jdk_string_filename additional_local( merged_settings.get("kernel.settings.additional.local" ) );
    jdk_string_url additional_remote( merged_settings.get("kernel.settings.additional.remote") );
    jdk_string_url http_proxy_auth_basic_username = merged_settings.get("http.proxy.auth.basic.username");
    jdk_string_url http_proxy_auth_basic_password = merged_settings.get("http.proxy.auth.basic.password");


    jdk_string_url http_proxy( merged_settings.get("http.proxy" ) );    

    if( !additional_remote.is_clear() )
    {
      jdk_remote_buf b( additional_local, additional_remote, http_proxy, 1024*1024, false, http_proxy_auth_basic_username, http_proxy_auth_basic_password, true );
      b.load_local();
      b.check_and_grab();
      additional_settings.load_buf( b.get_buf() );
    }
    return true;
  }

  void reload()
  {
    install_settings.clear();
    install_settings.copy_prefixed( default_settings, "http." );
    install_settings.copy_prefixed( default_settings, "httpfilter." );
    install_settings.copy_prefixed( default_settings, "kernel." );
    install_settings.copy_prefixed( default_settings, "log." );
    load_system_install();

    license_settings.clear();
    license_settings.copy_prefixed( default_settings, "license." );
    load_system_license();

    additional_settings.clear();
    additional_settings.copy_prefixed( default_settings, "kernel.blocking." );
    additional_settings.copy_prefixed( install_settings, "kernel.blocking." );
    additional_settings.copy_prefixed( default_settings, "blockpage." );
    additional_settings.copy_prefixed( install_settings, "blockpage." );
    additional_settings.copy_prefixed( default_settings, "kernel.override." );
    additional_settings.copy_prefixed( install_settings, "kernel.override." );
    additional_settings.copy_prefixed( default_settings, "kernel.db." );
    additional_settings.copy_prefixed( install_settings, "kernel.db." );
    additional_settings.copy_prefixed( default_settings, "kernel.predb." );
    additional_settings.copy_prefixed( install_settings, "kernel.predb." );
    additional_settings.copy_prefixed( default_settings, "kernel.settings." );
    additional_settings.copy_prefixed( install_settings, "kernel.settings." );
    additional_settings.copy_prefixed( default_settings, "kernel.log." );
    additional_settings.copy_prefixed( install_settings, "kernel.log." );

    load_system_additional();
    //load_remote_additional();
#if JDK_IS_WIN32
    {
      jdk_str<64> runtime_id;
      runtime_id.form( "%d-%d", GetTickCount()&0xffffff, clock()&0xffffff );
      args_settings.set( "runtime.id", runtime_id );
    } 

#else
    {
      jdk_str<64> runtime_id;
      runtime_id.form( "%d-%d", random()&0xffffff, random()&0xffffff );
      args_settings.set( "runtime.id", runtime_id );
    } 

#endif

    if2kd_validate_license( license_settings);
    save_system_license();

    merged_settings.clear();
    merged_settings.merge( default_settings );
    merged_settings.merge( install_settings );
    merged_settings.merge( additional_settings );
    merged_settings.merge( license_settings );
    merged_settings.merge( args_settings );
  }

  bool save_system_additional()
  {
#if JDK_IS_WIN32
    additional_settings.save_registry( HKEY_LOCAL_MACHINE, IF2K_MINI_ADDITIONAL_REGISTRY_LOCATION );
#else
    additional_settings.save_file(merged_settings.get("kernel.settings.additional.local") );
#endif
    return true;
  }

  bool save_system_install()
  {
#if JDK_IS_WIN32
    install_settings.save_registry( HKEY_LOCAL_MACHINE, IF2K_MINI_INSTALL_REGISTRY_LOCATION );
#else
    install_settings.save_file( merged_settings.get("kernel.settings.master.local") );
#endif
    return true;
  }

  bool save_system_license()
  {
#if JDK_IS_WIN32
    license_settings.save_registry( HKEY_LOCAL_MACHINE, IF2K_MINI_LICENSE_REGISTRY_LOCATION );
#else
    license_settings.save_file( merged_settings.get("kernel.settings.license.local") );
#endif
    return true;
  }

  bool save()
  {
    save_system_install();
    save_system_license();
    save_system_additional();
    return true;
  }

 int get_reload_time_in_seconds()
  {
    if( InternetGetConnectedState(0,0)==TRUE ) 
    {
      return merged_settings.get_long( "kernel.settings.update.rate")*60;
    }
    else
    {
      return merged_settings.get_long( "kernel.settings.refresh.rate")*60;
    }
  }
  
  jdk_settings &get_merged_settings()      { return merged_settings;     }
  jdk_settings &get_install_settings()     { return install_settings;    }
  jdk_settings &get_license_settings()     { return license_settings;    }
  jdk_settings &get_additional_settings()  { return additional_settings; }
  jdk_settings &get_args_settings()  { return args_settings; }

#if JDK_IS_WIN32
  jdk_settings_win32registry install_settings;
  jdk_settings_win32registry license_settings;
  jdk_settings_win32registry additional_settings;
#else
  jdk_settings install_settings;
  jdk_settings license_settings;
  jdk_settings additional_settings;
#endif
  jdk_settings args_settings;

  jdk_settings merged_settings;
  jdk_settings default_settings;
  
};



class if2k_mini_kernel_v2 : public if2k_mini_kernel
{
  explicit if2k_mini_kernel_v2( const if2k_mini_kernel_v2 & );
  const if2k_mini_kernel_v2 & operator = ( const if2k_mini_kernel_v2 &o );

public:
  explicit if2k_mini_kernel_v2(if2k_mini_kernel_v2_settings_collection &settings_collection_ );
  virtual ~if2k_mini_kernel_v2();

  virtual void load();

  virtual const jdk_settings &get_install_settings() { return settings_collection.install_settings; }
  virtual const jdk_settings &get_additional_settings() { return settings_collection.additional_settings; }
  virtual const jdk_settings &get_license_settings() { return settings_collection.license_settings; }

  virtual bool is_restart_triggered();
  virtual bool is_precompile_triggered();
  virtual bool is_in_override_mode();
  virtual bool set_override_mode( bool f );
  virtual bool update_install_settings( const jdk_settings &s ); 
  virtual bool update_additional_settings( const jdk_settings &s ); 
  virtual bool update_license_settings( const jdk_settings &s ); 
  virtual bool get_predb_file( const jdk_string_filename &f, jdk_buf &b );
  virtual bool get_db_file( const jdk_string_filename &f, jdk_buf &b );
  virtual bool set_predb_file( const jdk_string_filename &f, const jdk_buf &b );
  virtual bool set_db_file( const jdk_string_filename &f, const jdk_buf &b );
  virtual bool trigger_restart();
  virtual bool trigger_precompile();
  virtual void fix_url_list( jdk_dynbuf &bout, const jdk_buf &bin );


  virtual void verify_url( 
    if2k_mini_kernel_request &request    
    );

  virtual void verify_nntp_group(
    if2k_mini_kernel_request &request,
    jdk_string &group
    );

  virtual void verify_nntp_subject(
    if2k_mini_kernel_request &request,
    jdk_string &subject
    );

  virtual void verify_nntp_article(
    if2k_mini_kernel_request &request,
    jdk_buf &data
    );


  virtual void verify_received_data(
    if2k_mini_kernel_request &request,
    jdk_buf &data
    );

  virtual void verify_transmitted_data(
    if2k_mini_kernel_request &request,
    jdk_buf &data
    );


protected:


  if2k_mini_kernel_v2_settings_collection &settings_collection;
  jdk_settings &settings;

  bool override_allow;
  bool override_now;
  bool override_unknown_only;
  bool restart_triggered;
  bool precompile_triggered;

  bool categories_enable[8];
  jdk_str<256> categories_name[8];
  bool good_categories_enable[8];
  jdk_str<256> good_categories_name[8];
  bool blocking_enable;
  bool block_bad;
  bool block_unknown;
  bool block_bad_images;
  bool censor_bad_url_in_content;
  bool censor_bad_phrase_in_content;
  bool block_sites_with_bad_url_in_content;
  bool block_sites_with_bad_phrase_in_content;
  int content_match_threshold;
  jdk_str<4096> nntp_redirect_group;

private:
  if2k_mini_url_scanner_precompiled *precompiled_good_urls[8];
  if2k_mini_url_scanner *user_good_urls[8];

  if2k_mini_url_scanner_precompiled *precompiled_bad_urls[8];
  if2k_mini_url_scanner *user_bad_urls[8];

  if2k_mini_url_scanner_precompiled *precompiled_postbad_urls[8];
  if2k_mini_url_scanner *user_postbad_urls[8];

  if2k_mini_alphanumeric_scanner_precompiled *precompiled_bad_phrases[8];
  if2k_mini_alphanumeric_scanner *user_bad_phrases[8];

  if2k_mini_alphanumeric_scanner_precompiled *precompiled_bad_nntp[8];
  if2k_mini_alphanumeric_scanner *user_bad_nntp[8];

  if2k_mini_alphanumeric_scanner_precompiled *precompiled_good_nntp[8];
  if2k_mini_alphanumeric_scanner *user_good_nntp[8];
};




template <class tree_traits>
class if2k_mini_kernel_v2_event  : public if2k_mini_tree_event<tree_traits>
{
public:
	if2k_mini_kernel_v2_event()
  :
    best_match_item(),
    largest_match_value(),
    match_count()
	{
		best_match_item = -1;
		largest_match_value = -1;
    match_count = 0;
	}
		
	
	virtual void operator () ( const jdk_tree<tree_traits> &tree, typename tree_traits::index_t match_item )
	{
		typename tree_traits::flags_t value = tree.get(match_item).flags;
		
		if(value > largest_match_value )
		{
			best_match_item = match_item;
			largest_match_value = value;
		}
    match_count++;
	}	
	
	typename tree_traits::index_t best_match_item;
	typename tree_traits::flags_t largest_match_value;
  int match_count;
};

typedef if2k_mini_kernel_v2_event<if2k_mini_tree_traits_url> if2k_mini_kernel_v2_event_url;
typedef if2k_mini_kernel_v2_event<if2k_mini_tree_traits_alphanumeric> if2k_mini_kernel_v2_event_alphanumeric;
typedef if2k_mini_kernel_v2_event<if2k_mini_tree_traits_url_mmap> if2k_mini_kernel_v2_event_url_mmap;
typedef if2k_mini_kernel_v2_event<if2k_mini_tree_traits_alphanumeric_mmap> if2k_mini_kernel_v2_event_alphanumeric_mmap;


#if JDK_IS_VCPP



template <class SCANNER>
bool if2k_mini_kernel_v2_scan_bad( 
  SCANNER *scanner, 
  if2k_mini_kernel_request &request,
  const jdk_string &url_string
  )
{
  bool r=false;
  if( scanner )
  {
    if2k_mini_kernel_v2_event< SCANNER::tree_traits_t > ev;
    if( scanner->find( url_string.c_str(), url_string.len(), ev) )
    {
      jdk_setting_value match;
      int l=scanner->extract( match.c_str(), match.getmaxlen()-1, ev.best_match_item );
      match.set(l,'\0');

      jdk_log( JDK_LOG_DEBUG1, "URL Match in: %s category %s: %d %s", 
               scanner->get_tree_type().c_str(), 
               scanner->get_tree_category().c_str(),
               ev.best_match_item,
               match.c_str()
        );

      request.is_unknown = false;
      request.is_bad = true;

      request.set_category( scanner->get_tree_category().c_str() );
      request.set_matchtype( scanner->get_tree_type().c_str() );
      request.set_match( match );      

      r = true;
    }

  }

  return r;
}

template <class SCANNER>
bool if2k_mini_kernel_v2_scan_postbad( 
  SCANNER *scanner, 
  if2k_mini_kernel_request &request,
  const jdk_string &url_string
  )
{
  bool r=false;
  if( scanner )
  {
    if2k_mini_kernel_v2_event< SCANNER::tree_traits_t > ev;
    if( scanner->find( url_string.c_str(), url_string.len(), ev) )
    {
      jdk_setting_value match;
      int l=scanner->extract( match.c_str(), match.getmaxlen()-1, ev.best_match_item );
      match.set(l,'\0');

      jdk_log( JDK_LOG_DEBUG1, "URL Match in: %s category %s: %d %s", 
               scanner->get_tree_type().c_str(), 
               scanner->get_tree_category().c_str(),
               ev.best_match_item,
               match.c_str()
        );

      request.is_unknown = false;
      request.is_bad = true;
      request.is_postbad = true;
      request.set_category( scanner->get_tree_category().c_str() );
      request.set_matchtype( scanner->get_tree_type().c_str() );
      request.set_match( match );      

      r = true;
    }

  }

  return r;
}



template <class SCANNER>
bool if2k_mini_kernel_v2_scan_good( 
  SCANNER *scanner, 
  if2k_mini_kernel_request &request,
  const jdk_string &url_string
  )

{
  bool r=false;
  if( scanner )
  {
    if2k_mini_kernel_v2_event< SCANNER::tree_traits_t > ev;
    
    jdk_string_url fixed_url_string( url_string );
    // truncate url at any '?' or '&' character
    char *p='\0';
    p=fixed_url_string.chr('?');
    if( p )
    {
      *p='\0';
    }
    p=fixed_url_string.chr('&');
    if( p )
    {
      *p='\0';
    }

    if( scanner->find( fixed_url_string.c_str(), fixed_url_string.len(), ev) )
    {
      jdk_setting_value match;
      int l=scanner->extract( match.c_str(), match.getmaxlen()-1, ev.best_match_item );
      match.set(l,'\0');
      jdk_log( JDK_LOG_DEBUG1, "URL Match in: %s category %s: %d %s", 
               scanner->get_tree_type().c_str(), 
               scanner->get_tree_category().c_str(),
               ev.best_match_item,
               match.c_str()
         );


      request.is_unknown = false;
      request.is_good = true;

      request.set_category( scanner->get_tree_category().c_str() );
      request.set_matchtype( scanner->get_tree_type().c_str() );
      request.set_match( match );      

      r = true;
    }

  }

  return r;
}



template <class SCANNER>
bool if2k_mini_kernel_v2_scan_bad( 
  SCANNER *scanner, 
  if2k_mini_kernel_request &request,
  const jdk_buf &buf
  )
{
  bool r=false;
  if( scanner )
  {
    if2k_mini_kernel_v2_event< SCANNER::tree_traits_t > ev;
    if( scanner->find( buf, ev) )
    {
      jdk_setting_value match;
      int l=scanner->extract( match.c_str(), match.getmaxlen()-1, ev.best_match_item );

      match.set(l,'\0');

      jdk_log( JDK_LOG_DEBUG1, "DATA Match in: %s category %s: %d %s", 
               scanner->get_tree_type().c_str(), 
               scanner->get_tree_category().c_str(),
               ev.best_match_item,
               match.c_str()
        );



      request.is_unknown = false;
      request.is_bad = true;

      request.set_category( scanner->get_tree_category().c_str() );
      request.set_matchtype( scanner->get_tree_type().c_str() );
      request.set_match( match );      

      r = true;
    }
  }

  return r;
}



template <class SCANNER>
inline int if2k_mini_kernel_v2_censor( 
  SCANNER *scanner, 
  if2k_mini_kernel_request &request,
  jdk_buf &buf
  )
{
  int match_count=0;

  if( scanner )
  {
    if2k_mini_kernel_v2_event< SCANNER::tree_traits_t > ev;
    if( scanner->censor( buf, ev, '*' ) )
    {
      jdk_setting_value match;
      int l=scanner->extract( match.c_str(), match.getmaxlen()-1, ev.best_match_item );
      match.set(l,'\0');
      jdk_log( JDK_LOG_DEBUG1, "DATA CENSOR Match in: %s category %s: %d %s", 
               scanner->get_tree_type().c_str(), 
               scanner->get_tree_category().c_str(),
               ev.best_match_item,
               match.c_str()
        );

      match_count = ev.match_count;
      request.set_category( scanner->get_tree_category().c_str() );
      request.set_matchtype( scanner->get_tree_type().c_str() );
      request.set_match( match );      
    }
  }

  return match_count;

}



template <class SCANNER>
inline int if2k_mini_kernel_v2_censor( 
  SCANNER *scanner, 
  if2k_mini_kernel_request &request,
  jdk_string &s
  )
{
  int match_count=0;
  if( scanner )
  {
    if2k_mini_kernel_v2_event< SCANNER::tree_traits_t > ev;

    if( scanner->censor( s.c_str(), s.len(), ev, '*' ) )
    {
      jdk_setting_value match;
      int l=scanner->extract( match.c_str(), match.getmaxlen()-1, ev.best_match_item );

      match.set(l,'\0');

      jdk_log( JDK_LOG_DEBUG1, "DATA CENSOR Match in: %s category %s: %d %s", 
               scanner->get_tree_type().c_str(), 
               scanner->get_tree_category().c_str(),
               ev.best_match_item,
               match.c_str()
        );

      match_count = ev.match_count;
      request.set_category( scanner->get_tree_category().c_str() );
      request.set_matchtype( scanner->get_tree_type().c_str() );
      request.set_match( match );      
    }
  }
  return match_count;
}





#else
template <class SCANNER>
bool if2k_mini_kernel_v2_scan_bad( 
  SCANNER *scanner, 
  if2k_mini_kernel_request &request,
  const jdk_string &url_string
  )
{
  bool r=false;
  if( scanner )
  {
    if2k_mini_kernel_v2_event< typename SCANNER::tree_traits_t > ev;
    if( scanner->find( url_string.c_str(), url_string.len(), ev) )
    {
      jdk_setting_value match;
      size_t l=scanner->extract( match.c_str(), match.getmaxlen()-1, ev.best_match_item );
      match.set(l,'\0');

      jdk_log( JDK_LOG_DEBUG1, "URL Match in: %s category %s: %d %s", 
               scanner->get_tree_type().c_str(), 
               scanner->get_tree_category().c_str(),
               ev.best_match_item,
               match.c_str()
        );

      request.is_unknown = false;
      request.is_bad = true;
      request.set_category( scanner->get_tree_category().c_str() );
      request.set_matchtype( scanner->get_tree_type().c_str() );
      request.set_match( match );      

      r = true;
    }
  }
  return r;
}

template <class SCANNER>
bool if2k_mini_kernel_v2_scan_postbad( 
  SCANNER *scanner, 
  if2k_mini_kernel_request &request,
  const jdk_string &url_string
  )
{
  bool r=false;
  if( scanner )
  {
    if2k_mini_kernel_v2_event< typename SCANNER::tree_traits_t > ev;
    if( scanner->find( url_string.c_str(), url_string.len(), ev) )
    {
      jdk_setting_value match;
      size_t l=scanner->extract( match.c_str(), match.getmaxlen()-1, ev.best_match_item );
      match.set(l,'\0');

      jdk_log( JDK_LOG_DEBUG1, "URL Match in: %s category %s: %d %s", 
               scanner->get_tree_type().c_str(), 
               scanner->get_tree_category().c_str(),
               ev.best_match_item,
               match.c_str()
        );

      request.is_unknown = false;
      request.is_bad = true;
      request.is_postbad = true;
      request.set_category( scanner->get_tree_category().c_str() );
      request.set_matchtype( scanner->get_tree_type().c_str() );
      request.set_match( match );      

      r = true;
    }
  }
  return r;
}

template <class SCANNER>
bool if2k_mini_kernel_v2_scan_good( 
  SCANNER *scanner, 
  if2k_mini_kernel_request &request,
  const jdk_string &url_string
  )
{
  bool r=false;
  if( scanner )
  {
    if2k_mini_kernel_v2_event< typename SCANNER::tree_traits_t > ev;

    jdk_string_url fixed_url_string( url_string );
    // truncate url at any '?' or '&' character
    char *p='\0';
    p=fixed_url_string.chr('?');
    if( p )
    {
      *p='\0';
    }
    p=fixed_url_string.chr('&');
    if( p )
    {
      *p='\0';
    }

    if( scanner->find( fixed_url_string.c_str(), fixed_url_string.len(), ev) )
    {
      jdk_setting_value match;
      size_t l=scanner->extract( match.c_str(), match.getmaxlen()-1, ev.best_match_item );
      match.set(l,'\0');
      jdk_log( JDK_LOG_DEBUG1, "URL Match in: %s category %s: %d %s", 
               scanner->get_tree_type().c_str(), 
               scanner->get_tree_category().c_str(),
               ev.best_match_item,
               match.c_str()
        );

      request.is_unknown = false;
      request.is_good = true;

      request.set_category( scanner->get_tree_category().c_str() );
      request.set_matchtype( scanner->get_tree_type().c_str() );
      request.set_match( match );      

      r = true;
    }
  }
  return r;
}

template <class SCANNER>
bool if2k_mini_kernel_v2_scan_bad( 
  SCANNER *scanner, 
  if2k_mini_kernel_request &request,
  const jdk_buf &buf
  )
{
  bool r=false;
  if( scanner )
  {
    if2k_mini_kernel_v2_event< typename SCANNER::tree_traits_t > ev;
    if( scanner->find( buf, ev) )
    {
      jdk_setting_value match;
      int l=scanner->extract( match.c_str(), match.getmaxlen()-1, ev.best_match_item );
      match.set(l,'\0');
      jdk_log( JDK_LOG_DEBUG1, "DATA Match in: %s category %s: %d %s", 
               scanner->get_tree_type().c_str(), 
               scanner->get_tree_category().c_str(),
               ev.best_match_item,
               match.c_str()
        );

      request.is_unknown = false;
      request.is_bad = true;
      request.set_category( scanner->get_tree_category().c_str() );
      request.set_matchtype( scanner->get_tree_type().c_str() );
      request.set_match( match );      

      r = true;
    }
  }
  return r;
}

template <class SCANNER>
inline int if2k_mini_kernel_v2_censor( 
  SCANNER *scanner, 
  if2k_mini_kernel_request &request,
  jdk_buf &buf
  )
{
  int match_count=0;
  if( scanner )
  {
    if2k_mini_kernel_v2_event< typename SCANNER::tree_traits_t > ev;
    if( scanner->censor( buf, ev, '*' ) )
    {
      jdk_setting_value match;
      size_t l=scanner->extract( match.c_str(), match.getmaxlen()-1, ev.best_match_item );
      match.set(l,'\0');
      jdk_log( JDK_LOG_DEBUG1, "DATA CENSOR Match in: %s category %s: %d %s", 
               scanner->get_tree_type().c_str(), 
               scanner->get_tree_category().c_str(),
               ev.best_match_item,
               match.c_str()
        );
      match_count = ev.match_count;
      request.set_category( scanner->get_tree_category().c_str() );
      request.set_matchtype( scanner->get_tree_type().c_str() );
      request.set_match( match );      

    }
  }
  return match_count;
}

template <class SCANNER>
inline int if2k_mini_kernel_v2_censor( 
  SCANNER *scanner, 
  if2k_mini_kernel_request &request,
  jdk_string &s
  )
{
  int match_count=0;
  if( scanner )
  {
    if2k_mini_kernel_v2_event< typename SCANNER::tree_traits_t > ev;
    if( scanner->censor( s.c_str(), s.len(), ev, '*' ) )
    {
      jdk_setting_value match;
      int l=scanner->extract( match.c_str(), match.getmaxlen()-1, ev.best_match_item );
      match.set(l,'\0');
      jdk_log( JDK_LOG_DEBUG1, "DATA CENSOR Match in: %s category %s: %d %s", 
               scanner->get_tree_type().c_str(), 
               scanner->get_tree_category().c_str(),
               ev.best_match_item,
               match.c_str()
        );
      match_count = ev.match_count;
      request.set_category( scanner->get_tree_category().c_str() );
      request.set_matchtype( scanner->get_tree_type().c_str() );
      request.set_match( match );      
    }
  }
  return match_count;
}
#endif

#endif
