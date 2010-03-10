/* -*- mode: C; mode: fold; -*- */
#ifndef _JDK_URL_H
#define _JDK_URL_H

#include "jdk_string.h"
#include "jdk_cgi.h"
#include "jdk_cgisettings.h"

struct jdk_url
{
	jdk_url() : protocol(), host(), port(80), path()
	{		
	}
    
  jdk_url( const char *full_url ): protocol(), host(), port(80), path()
  {
    explode( full_url );
  }
    
  jdk_url( const jdk_string &full_url ): protocol(), host(), port(80), path()
  {
    explode( full_url );
  }

	void clear()
	{
		protocol.clear();
		host.clear();
		port=80;
		path.clear();
	}

  static jdk_string_url normalize_and_unescape( const jdk_string &u )
  {
    jdk_url url( u );
    jdk_string_url url_string;
    jdk_cgi_unescape( url.unexplode().c_str(), url_string.c_str(), url_string.getmaxlen() );
    return url_string;
  }

  jdk_string_url normalize_and_unescape() const
  {
    jdk_string_url url_string;
    jdk_cgi_unescape( unexplode().c_str(), url_string.c_str(), url_string.getmaxlen() );
    return url_string;
  }

  void unescape();
  void escape();

	int explode( 
				const char *full_url
				);
	int explode( 
				const jdk_string &full_url
				)
	{
	    return explode( full_url.c_str() );
	}
	
	jdk_str<1024> get_host_and_port() const
	{
		jdk_str<1024> hp;
		int p =port;
		if( p==-1 )
		  p=80;
		hp.form( "%s:%d", host.c_str(), p );
		return hp;
	}
	
  bool get_cgi_params( jdk_settings &t )
  {
    bool r=false;
    const char *p = path.chr('?');
    if( p )
    {
      r=jdk_cgi_loadsettings( &t, p );
    }
    else
    {
      t.clear();
    }
    return r;
  }
				
	jdk_str<4096> unexplode() const;

  jdk_str<64> protocol;
	jdk_str<256> host;
  int port;
	jdk_str<4096> path;	
};

#endif
