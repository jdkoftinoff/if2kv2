#include "jdk_world.h"
#include "jdk_stream.h"

bool jdk_stream_style_text::read_start_tag( const char * )
{
  return true;
}

bool jdk_stream_style_text::read_end_tag( const char *)
{
  return true;
}

bool jdk_stream_style_text::write_start_tag( const char * )
{
  return true;
}

bool jdk_stream_style_text::write_end_tag( const char * )
{
  return true;
}

bool jdk_stream_style_text::read( jdk_string &result )
{
  return stream->read( result );
}


bool jdk_stream_style_text::write( const jdk_string &val )
{
  return stream->write( val );
}


bool jdk_stream_style_text::read( jdk_dynbuf &result )
{
  return stream->read( result );
}


bool jdk_stream_style_text::write( const jdk_dynbuf &val )
{
  return stream->write( val );
}


bool jdk_stream_style_text::read( signed char &result )
{
  return stream->read( &result, 1 );
}


bool jdk_stream_style_text::write( const signed char &val )
{
  return stream->write( &val, 1 );
}


bool jdk_stream_style_text::read( unsigned char &result )
{
  return stream->read( &result, 1 );
}


bool jdk_stream_style_text::write( const unsigned char &val )
{
  return stream->write( &val, 1 );
}


bool jdk_stream_style_text::read( int16 &result )
{
  jdk_str<1024> buf;
  bool r=read(buf);
  if( r )
  {
    result=(int16)strtol(buf.c_str(),0,10);
  }
  return r;
}


bool jdk_stream_style_text::write( const int16 &val )
{
  jdk_str<32> buf;
  buf.form( "%d", val );
  return write(buf);
}


bool jdk_stream_style_text::read( uint16 &result )
{
  jdk_str<1024> buf;
  bool r=read(buf);
  if( r )
  {
    result=(uint16)strtoul(buf.c_str(),0,10);
  }
  return r;
}


bool jdk_stream_style_text::write( const uint16 &val )
{
  jdk_str<32> buf;
  buf.form( "%ud", val );
  return write(buf);
}


bool jdk_stream_style_text::read( int32 &result )
{
  jdk_str<1024> buf;
  bool r=read(buf);
  if( r )
  {
    result=strtol(buf.c_str(),0,10);
  }
  return r;
}


bool jdk_stream_style_text::write( const int32 &val )
{
  jdk_str<32> buf;
  buf.form( "%ld", val );
  return write(buf);
}


bool jdk_stream_style_text::read( uint32 &result )
{
  jdk_str<1024> buf;
  bool r=read(buf);
  if( r )
  {
    result=strtoul(buf.c_str(),0,10);
  }
  return r;
  
}


bool jdk_stream_style_text::write( const uint32 &val )
{
  jdk_str<32> buf;
  buf.form( "%lu", val );
  return write(buf);
}


bool jdk_stream_style_text::read( float &result )
{
  jdk_str<1024> buf;
  bool r=read(buf);
  if( r )
  {
    result=(float)strtod(buf.c_str(),0);
  }
  return r;	
}


bool jdk_stream_style_text::write( const float &val )
{
  jdk_str<32> buf;
  buf.form( "%g", (double)val );
  return write(buf);
}


bool jdk_stream_style_text::read( double &result )
{
  jdk_str<1024> buf;
  bool r=read(buf);
  if( r )
  {
    result=strtod(buf.c_str(),0);
  }
  return r;	
}


bool jdk_stream_style_text::write( const double &val )
{
  jdk_str<32> buf;
  buf.form( "%g", val );
  return write(buf);
}



#if 0 // TODO:

bool jdk_stream_style_bin::read_start_tag( const char * )
{
  return true;
}

bool jdk_stream_style_bin::read_end_tag( const char *)
{
  return true;
}

bool jdk_stream_style_bin::write_start_tag( const char * )
{
  return true;
}

bool jdk_stream_style_bin::write_end_tag( const char * )
{
  return true;
}

bool jdk_stream_style_bin::read( jdk_string &result )
{
  
}


bool jdk_stream_style_bin::write( const jdk_string &val )
{
  
}


bool jdk_stream_style_bin::read( jdk_dynbuf &result )
{
  
}


bool jdk_stream_style_bin::write( const jdk_dynbuf &val )
{
  
}


bool jdk_stream_style_bin::read( signed char &result )
{
  
}


bool jdk_stream_style_bin::write( const signed char &val )
{
  
}


bool jdk_stream_style_bin::read( unsigned char &result )
{
  
}


bool jdk_stream_style_bin::write( const unsigned char &val )
{
  
}


bool jdk_stream_style_bin::read( int16 &result )
{
  
}


bool jdk_stream_style_bin::write( const int16 &val )
{
  
}


bool jdk_stream_style_bin::read( uint16 &result )
{
  
}


bool jdk_stream_style_bin::write( const uint16 &val )
{
  
}


bool jdk_stream_style_bin::read( int32 &result )
{
  
}


bool jdk_stream_style_bin::write( const int32 &val )
{
  
}


bool jdk_stream_style_bin::read( uint32 &result )
{
  
}


bool jdk_stream_style_bin::write( const uint32 &val )
{
  
}


bool jdk_stream_style_bin::read( float &result )
{
  
}


bool jdk_stream_style_bin::write( const float &val )
{
  
}


bool jdk_stream_style_bin::read( double &result )
{
  
}


bool jdk_stream_style_bin::write( const double &val )
{
  
}



#endif
///////////


bool jdk_stream_style_xml::read_start_tag( const char *tag )
{
  bool r=true;
  if( tag )
  {
    jdk_str<128> s;
    r=read(s);
    if( r )
    {
      jdk_str<128> t;
      t.form( "<%s>", tag );
      if( s.cmp(t)!=0 )
        r=false;
    }
  }
  return r;
}

bool jdk_stream_style_xml::read_end_tag( const char *tag)
{
  bool r=true;
  if( tag )
  {
    jdk_str<128> s;
    r=read(s);
    if( r )
    {
      jdk_str<128> t;
      t.form( "</%s>", tag );
      if( s.cmp(t)!=0 )
        r=false;
    }
  }
  return r;
}

bool jdk_stream_style_xml::write_start_tag( const char *tag )
{
  bool r=true;
  if( tag )
  {
    jdk_str<128> s;
    s.form( "<%s>", tag);
    r=write(s);
  }
  return r;
}

bool jdk_stream_style_xml::write_end_tag( const char *tag )
{
  bool r=true;
  if( tag )
  {
    jdk_str<128> s;
    s.form( "</%s>", tag );
    r=write(s);
  }
  return r;
}

bool jdk_stream_style_xml::read( jdk_string &result )
{
  return stream->read( result );
}


bool jdk_stream_style_xml::write( const jdk_string &val )
{
  return stream->write( val );
}


bool jdk_stream_style_xml::read( jdk_dynbuf &result )
{
  return stream->read( result );
}


bool jdk_stream_style_xml::write( const jdk_dynbuf &val )
{
  return stream->write( val );
}


bool jdk_stream_style_xml::read( signed char &result )
{
  return stream->read( &result, 1 );
}


bool jdk_stream_style_xml::write( const signed char &val )
{
  return stream->write( &val, 1 );
}


bool jdk_stream_style_xml::read( unsigned char &result )
{
  return stream->read( &result, 1 );
}


bool jdk_stream_style_xml::write( const unsigned char &val )
{
  return stream->write( &val, 1 );
}


bool jdk_stream_style_xml::read( int16 &result )
{
  jdk_str<1024> buf;
  bool r=read(buf);
  if( r )
  {
    result=(int16)strtol(buf.c_str(),0,10);
  }
  return r;
}


bool jdk_stream_style_xml::write( const int16 &val )
{
  jdk_str<32> buf;
  buf.form( "%d", val );
  return write(buf);
}


bool jdk_stream_style_xml::read( uint16 &result )
{
  jdk_str<1024> buf;
  bool r=read(buf);
  if( r )
  {
    result=(uint16)strtoul(buf.c_str(),0,10);
  }
  return r;
}


bool jdk_stream_style_xml::write( const uint16 &val )
{
  jdk_str<32> buf;
  buf.form( "%ud", val );
  return write(buf);
}


bool jdk_stream_style_xml::read( int32 &result )
{
  jdk_str<1024> buf;
  bool r=read(buf);
  if( r )
  {
    result=strtol(buf.c_str(),0,10);
  }
  return r;
}


bool jdk_stream_style_xml::write( const int32 &val )
{
  jdk_str<32> buf;
  buf.form( "%ld", val );
  return write(buf);
}


bool jdk_stream_style_xml::read( uint32 &result )
{
  jdk_str<1024> buf;
  bool r=read(buf);
  if( r )
  {
    result=strtoul(buf.c_str(),0,10);
  }
  return r;
  
}


bool jdk_stream_style_xml::write( const uint32 &val )
{
  jdk_str<32> buf;
  buf.form( "%lu", val );
  return write(buf);
}


bool jdk_stream_style_xml::read( float &result )
{
  jdk_str<1024> buf;
  bool r=read(buf);
  if( r )
  {
    result=(float)strtod(buf.c_str(),0);
  }
  return r;	
}


bool jdk_stream_style_xml::write( const float &val )
{
  jdk_str<32> buf;
  buf.form( "%g", (double)val );
  return write(buf);
}


bool jdk_stream_style_xml::read( double &result )
{
  jdk_str<1024> buf;
  bool r=read(buf);
  if( r )
  {
    result=strtod(buf.c_str(),0);
  }
  return r;	
}


bool jdk_stream_style_xml::write( const double &val )
{
  jdk_str<32> buf;
  buf.form( "%g", val );
  return write(buf);
}








