#ifndef _IF2K_MINI_ClIENT_H
#define _IF2K_MINI_ClIENT_H

#include "jdk_world.h"
#include "jdk_settings.h"
#include "jdk_dynbuf.h"
#include "jdk_string.h"
#include "jdk_httprequest.h"

#include "if2k_mini_config.h"

class if2k_mini_client
{
public:
  if2k_mini_client()
    {
      password.cpy( "password" );
      server_address.cpy( "localhost:8000" );
    }

  virtual ~if2k_mini_client()
    {
    }

  void set_password( const char *p ) { password.cpy(p); }
  void set_server_address( const char *s ) { server_address.cpy(s); }

  bool get_password( jdk_string &p ) { p.cpy( password ); return true; }
  bool get_server_address( jdk_string &s ) { s.cpy( server_address ); return true; }
  
  bool get_override_mode( bool &override )
    {      
      bool r=false;
      jdk_settings s;
      if( do_get_settings( s, "cgi/override.cgi" ) )
      {
        override = s.get_bool( "override" );
        r=true;
      }
      return r;
    }

  bool get_install_settings( jdk_settings &s )
    {
      return do_get_settings( s, "cgi/settings.cgi", "install" );
    }

  bool get_additional_settings( jdk_settings &s )
    {
      bool r =do_get_settings( s, "cgi/settings.cgi", "additional" );
      return r;
    }

  bool get_license_settings( jdk_settings &s )
    {
      return do_get_settings( s, "cgi/settings.cgi", "license" );
    }

  bool get_dbfile( const char *fn, jdk_dynbuf &b )
    {
      return do_get( b, "cgi/db.cgi", fn );
    }

  bool get_predbfile( const char *fn, jdk_dynbuf &b )
    {
      return do_get( b, "cgi/predb.cgi", fn );
    }

  bool post_override_mode( bool override )
    {
      jdk_settings s;
      s.set_bool( "override", override );

      return do_post_settings( s, "cgi/override.cgi" );
    }

  bool post_install_settings( const jdk_settings &s )
    {
      return do_post_settings( s, "cgi/settings.cgi", "install" );
    }

  bool post_additional_settings( const jdk_settings &s )
    {
      printf( "pre: %s\n", s.get( "kernel.settings.update.rate" ).c_str() );

      return do_post_settings( s, "cgi/settings.cgi", "additional" );
    }

  bool post_license_settings( const jdk_settings &s )
    {
      return do_post_settings( s, "cgi/settings.cgi", "license" );
    }

  bool post_dbfile( const char *fn, jdk_dynbuf &b )
    {
      return do_post( b, "cgi/db.cgi", fn );
    }

  bool post_predbfile( const char *fn, jdk_dynbuf &b )
    {
      return do_post( b, "cgi/predb.cgi", fn );
    }

  bool trigger_restart()
    {
      jdk_dynbuf dum;
      return do_get( dum, "cgi/restart.cgi" );
    }

  bool trigger_precompile()
    {
      jdk_dynbuf dum;
      return do_get( dum, "cgi/precompile.cgi" );
    }

private:

  bool do_get_settings( jdk_settings &s, const char *cgi, const char *f=0 )
    {
      bool r=false;
      jdk_dynbuf buf;
      if( do_get( buf, cgi, f ) )
      {
        s.clear();
        s.load_buf( buf );
        r=true;
      }
      return r;
    }

  bool do_post_settings( const jdk_settings &s, const char *cgi, const char *f=0 )
    {
      bool r=false;
      jdk_dynbuf buf;
      s.save_buf( &buf );

      if( do_post( buf, cgi, f ) )
      {
        r=true;
      }
      return r;
    }

  bool do_get( jdk_dynbuf &buf, const char *cgi, const char *f=0 )
    {
      jdk_string_url url( form_url( cgi, f) );

      jdk_http_response_header response_header;
      buf.clear();
      
      int r = jdk_http_get(
        url.c_str(),
        &buf,
        1024*1024,
        &response_header,
        0,
        false,
        0
        );
      
      return r==200;
    }

  bool do_post( jdk_dynbuf &buf, const char *cgi, const char *f=0 )
    {
      jdk_string_url url( form_url( cgi, f) );

      jdk_http_response_header response_header;
      jdk_dynbuf response;

      int r = jdk_http_post(
        url.c_str(),
        buf,
        "text/plain",
        &response,
        1024*1024,
        &response_header,
        0,
        false,
        0
        );
      return r==200;
    }

  jdk_string_url form_url( const char *cgi, const char *f )
    {
      jdk_settings params;

      params.set( "p", password );
      if( f )
      {
        params.set( "f", f );
      }

      jdk_string_url params_url;
      jdk_cgi_savesettings( params, params_url.c_str(), params_url.getmaxlen() );

      jdk_string_url url;
      url.form("http://%s/%s?%s", server_address.c_str(), cgi, params_url.c_str() );
      return url;
    }

  jdk_str<256> password;
  jdk_str<256> server_address;
};


class if2k_mini_client_collection
{
public:
  if2k_mini_client_collection( if2k_mini_client &if2k_ )
    : if2k( if2k_ )
  {
  }

  ~if2k_mini_client_collection()
  {
  }

  bool get_all_settings()
  {
    bool r =
      if2k.get_install_settings( install_settings ) &&
      if2k.get_additional_settings( additional_settings ) &&
      if2k.get_license_settings( license_settings );

    int i;
    for( i=0; i<8 && r==true; ++i )
    {
      jdk_string_filename f;
      f.form("%02dbadurl.txt", i+1 );
      r = if2k.get_dbfile( f.c_str(), badurl[i] );

      if( r )
      {
        f.form("%02dpostbadurl.txt", i+1 );
        r = if2k.get_dbfile( f.c_str(), postbadurl[i] );
      }


      if( r )
      {
        f.form("%02dgoodurl.txt", i+1 );
        r = if2k.get_dbfile( f.c_str(), goodurl[i] );
      }

      if( r )
      {
        f.form("%02dbadphr.txt", i+1 );
        r = if2k.get_dbfile( f.c_str(), badphr[i] );
      }
    }

    return r;
  }

  bool send_all_settings()
  {
    bool r =
      if2k.post_install_settings( install_settings ) &&
      if2k.post_additional_settings( additional_settings ) &&
      if2k.post_license_settings( license_settings );

    int i;
    for( i=0; i<8 && r==true; ++i )
    {
      jdk_string_filename f;
      f.form("%02dbadurl.txt", i+1 );
      r = if2k.post_dbfile( f.c_str(), badurl[i] );

      if( r )
      {
        f.form("%02dpostbadurl.txt", i+1 );
        r = if2k.post_dbfile( f.c_str(), postbadurl[i] );
      }

      if( r )
      {
        f.form("%02dgoodurl.txt", i+1 );
        r = if2k.post_dbfile( f.c_str(), goodurl[i] );
      }

      if( r )
      {
        f.form("%02dbadphr.txt", i+1 );
        r = if2k.post_dbfile( f.c_str(), badphr[i] );
      }
    }
    return r;
  }

  if2k_mini_client &if2k;

  jdk_settings license_settings;
  jdk_settings install_settings;
  jdk_settings additional_settings;

  jdk_dynbuf badurl[8];
  jdk_dynbuf postbadurl[8];
  jdk_dynbuf goodurl[8];
  jdk_dynbuf badphr[8];
};

#endif
