/* -*- mode: C; mode: fold; -*- */
#include "jdk_world.h"
#include "jdk_url.h"
#include "jdk_cgi.h"

void jdk_url::unescape()
{
  jdk_cgi_unescape( host );
  jdk_cgi_unescape( path );
}

void jdk_url::escape()
{
  jdk_str<4096> newpath;
  jdk_cgi_escape( path, newpath );
  path.cpy( newpath );
}



jdk_str<4096> jdk_url::unexplode() const 
{
  jdk_str<4096> buf;
  if( port==-1 || port==80 )
  {	
    if( !protocol.is_clear() && !host.is_clear() )
    {
      buf.form("%s://%s%s", protocol.c_str(), host.c_str(), path.c_str());
    }
    else
    {
      buf.cpy( path );
    }
  }
  else
  {   
    buf.form( "%s://%s:%d%s", protocol.c_str(), host.c_str(), port, path.c_str() );
  }	
  return buf;
} 


int jdk_url::explode( 
  const char *full_url
  ) 
{
  int r=-1;
  
  port = -1;
  
  if( *full_url=='/' )
  {
    host.clear();
    port=80;
    path.cpy(full_url);
    protocol.cpy("http");
    return 1;
  }
  
  // check for url in the form
  //  http://hostname:80/pathpathpath#section
  
  
  r=jdk_sscanf( full_url, "%63[^:]://%255[^:/]:%d%4095[^#]", protocol.c_str(), host.c_str(), &port, path.c_str() );
  
  if( r<4 || host.chr('/')!=0 )
  {
    // check for url in the form
    // http://hostname/pathpathpath#section
    
    r=jdk_sscanf( full_url, "%63[^:]://%255[^/]%4095[^#]", protocol.c_str(), host.c_str(), path.c_str() );
    
    if( r<3 )
    {
      // check for url in the form
      // http://hostname:port
      r=jdk_sscanf( full_url, "%63[^:]://%255[^:/]%d", protocol.c_str(), host.c_str(),  &port );
      if( r==3 )
      {
        path = "";
        return 1;
      }
      
      // check for url in the form
      // http://hostname
      r=jdk_sscanf( full_url, "%63[^:]://%255[^/]", protocol.c_str(), host.c_str() );
      if( r==2 )
      {
        path = "";
        port=80;
        return 1;
      }
      
      // check for url in the form
      // hostname:port/path#section
      
      r=jdk_sscanf( full_url, "%255[^:/]:%d/%4095[^#]", host.c_str(), &port, path.c_str() );
      if( r== 3 )
      {
        if( port==443 )
          protocol = "https";
        else
          protocol = "http";
        path.prepend('/');
        return 1;
      }
      
      // check for url in the form
      // hostname/path#section
      
      r=jdk_sscanf( full_url, "%255[^:/]/%4095[^#]", host.c_str(), path.c_str() );
      if( r== 2 )
      {
        port=80;
        protocol = "http";
        path.prepend('/');
        return 1;
      }
      
      
      // check for url in the form
      // hostname:port
      
      r=jdk_sscanf( full_url, "%255[^:/]:%d", host.c_str(), &port );
      if( r== 2 )
      {
        if( port==443 )
          protocol = "https";
        else
          protocol = "http";
        
        path = "/";
        return 1;
      }
      
      // check for url in the form
      // hostname
      r=jdk_sscanf( full_url, "%255[^:/][^#]", host.c_str() );
      if( r<1 )
        return 0;
      
      protocol = "http";
      
      path = "/";
      port=80;
      return 1;
    }
    
  }
  
  //path.prepend('/');
  
  if( port==-1 )
  {
    if( protocol.icmp( "http" )==0 )
      port = 80;
    else if( protocol.icmp( "https" )==0 )
      port = 443;
  }
  
  
  return 1;
}










