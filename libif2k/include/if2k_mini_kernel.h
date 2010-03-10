#ifndef __IF2K_MINI_KERNEL_H
#define __IF2K_MINI_KERNEL_H

#include "jdk_settings.h"

#include "if2k_mini_config.h"

struct if2k_mini_kernel_request
{
  if2k_mini_kernel_request() 
  :
    is_good(false), is_bad(false), is_postbad( false ), is_unknown(true), is_blocked(false), override(false)
  {
    info.set("url","");
    info.set("reason","");
    info.set("category","0");
    info.set("match","");
    info.set("matchtype","");
    info.set("client","");
  }

  if2k_mini_kernel_request( const if2k_mini_kernel_request &o )
    : 
    info(o.info),
    is_good(o.is_good),
    is_bad(o.is_bad),
    is_postbad( o.is_postbad ),
    is_unknown(o.is_unknown),
    is_blocked(o.is_blocked),
    override(o.override)
  {
  }

  const if2k_mini_kernel_request & operator = ( const if2k_mini_kernel_request &o )
  {
    info = o.info;
    is_good = o.is_good;
    is_bad = o.is_bad;
    is_postbad = o.is_postbad;
    is_unknown = o.is_unknown;
    is_blocked = o.is_blocked;
    override = o.override;
    return *this;
  }

  void set_url( const jdk_string &url_ )
  { 
    info.set("url", url_ );
  }

  void set_url( const char *url_ )
  { 
    info.set("url", url_ );
  }

  jdk_string_url get_url() const
  { 
    return info.get("url");
  }

  void set_matchtype( const jdk_string &m )
  { 
    info.set("matchtype", m );
  }

  void set_matchtype( const char *m )
  { 
    info.set("matchtype", m );
  }

  jdk_setting_value get_matchtype() const
  { 
    return info.get("matchtype");
  }


  const jdk_settings & get_info() const
  {
    return info;
  }

  jdk_settings & get_info()
  {
    return info;
  }

  void set_reason( const jdk_string &r )
  {
    info.set( "reason", r.c_str() );
  }

  void set_reason( const char *s )
  {
    info.set( "reason", s );
  }

  jdk_setting_value get_reason() const
  {
    return info.get( "reason" );
  }

  void set_category( const jdk_string &s )
  {
    info.set( "category", s );
  }

  void set_category( const char *s )
  {
    info.set( "category", s );
  }

  const jdk_string &get_category() const
  {
    return info.get( "category" );
  }

  void set_match( const jdk_string &r )
  {
    info.set( "match", r.c_str() );
  }

  void set_match( const char *s )
  {
    info.set( "match", s );
  }

  jdk_setting_value get_match() const
  {
    return info.get( "match" );
  }

  void set_client( const jdk_string &r )
  {
    info.set( "client", r.c_str() );
  }

  void set_client( const char *s )
  {
    info.set( "client", s );
  }

  jdk_setting_value get_client() const
  {
    return info.get( "client" );
  }

  jdk_settings info;
  bool is_good;
  bool is_bad;
  bool is_postbad;
  bool is_unknown;
  bool is_blocked;
  bool override;
};


class if2k_mini_kernel 
{
  if2k_mini_kernel( const if2k_mini_kernel & );
  const if2k_mini_kernel & operator = ( const if2k_mini_kernel &o );

public:
  if2k_mini_kernel() {}
  virtual ~if2k_mini_kernel() {}

  virtual void load() = 0;

  virtual const jdk_settings &get_install_settings() = 0;
  virtual const jdk_settings &get_additional_settings() = 0;
  virtual const jdk_settings &get_license_settings() = 0;

  virtual bool is_restart_triggered() = 0;
  virtual bool is_precompile_triggered() = 0;
  virtual bool is_in_override_mode() = 0;
  virtual bool set_override_mode( bool f ) = 0;
  virtual bool update_install_settings( const jdk_settings &s ) = 0; 
  virtual bool update_additional_settings( const jdk_settings &s ) = 0; 
  virtual bool update_license_settings( const jdk_settings &s ) = 0; 
  virtual bool get_predb_file( const jdk_string_filename &f, jdk_buf &b ) = 0;
  virtual bool get_db_file( const jdk_string_filename &f, jdk_buf &b ) = 0;
  virtual bool set_predb_file( const jdk_string_filename &f, const jdk_buf &b ) = 0;
  virtual bool set_db_file( const jdk_string_filename &f, const jdk_buf &b ) = 0;
  virtual bool trigger_restart() = 0;
  virtual bool trigger_precompile() = 0;
  virtual void fix_url_list( jdk_dynbuf &bout, const jdk_buf &bin ) = 0;

  virtual void verify_url( 
     if2k_mini_kernel_request &request    
    ) = 0;

  virtual void verify_nntp_group(
     if2k_mini_kernel_request &request,
    jdk_string &group
    ) = 0;

  virtual void verify_nntp_subject(
     if2k_mini_kernel_request &request,
    jdk_string &subject
    ) = 0;

  virtual void verify_nntp_article(
     if2k_mini_kernel_request &request,
    jdk_buf &data
    ) = 0;


  virtual void verify_received_data(
     if2k_mini_kernel_request &request,
    jdk_buf &data
    ) = 0;

  virtual void verify_transmitted_data(
     if2k_mini_kernel_request &request,
    jdk_buf &data
    ) = 0;
protected:
};


class if2k_mini_kernel_dummy : public if2k_mini_kernel
{
  explicit if2k_mini_kernel_dummy( const if2k_mini_kernel_dummy & );
  const if2k_mini_kernel_dummy & operator = ( const if2k_mini_kernel_dummy &o );

public:
  explicit if2k_mini_kernel_dummy() {}
  virtual ~if2k_mini_kernel_dummy() {}

  virtual void load() {}

  virtual void verify_url( 
     if2k_mini_kernel_request &request    
    )
    {
    }

  virtual void verify_nntp_group(
     if2k_mini_kernel_request &request,
    jdk_string &group
    )
    {
    }

  virtual void verify_nntp_subject(
     if2k_mini_kernel_request &request,
    jdk_string &subject
    )
    {
    }

  virtual void verify_nntp_article(
     if2k_mini_kernel_request &request,
    jdk_buf &data
    )
    {
    }


  virtual void verify_received_data(
     if2k_mini_kernel_request &request,
    jdk_buf &data
    )
    {
    }

  virtual void verify_transmitted_data(
     if2k_mini_kernel_request &request,
    jdk_buf &data
    )
    {
    }

protected:
};


#endif
