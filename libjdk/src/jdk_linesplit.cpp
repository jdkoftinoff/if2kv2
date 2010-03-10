#include "jdk_world.h"
#include "jdk_linesplit.h"

jdk_linesplit::jdk_linesplit( size_t line_sz_ ) :
  buf(0),
  buf_len(0),
  buf_pos(0),
  line( new char[line_sz_] ),
  line_sz( line_sz_ ),
  line_pos(0)
{
}

jdk_linesplit::jdk_linesplit( const jdk_linesplit &o ) : 
  buf(0),
  buf_len(0),
  buf_pos(0),
  line( new char[o.line_sz] ),
  line_sz( o.line_sz ),
  line_pos( o.line_pos )
{
  memcpy( line, o.line, o.line_pos );
}

const jdk_linesplit & jdk_linesplit::operator = ( const jdk_linesplit &o )
{
  buf = 0;
  buf_len = 0;
  buf_pos = 0;
  line = new char[o.line_sz];
  line_sz = o.line_sz;
  line_pos = o.line_pos;
  memcpy( line, o.line, o.line_pos );
  return *this;
}

jdk_linesplit::~jdk_linesplit()
{
  delete [] line;	
}

void jdk_linesplit::set_buf( char *buf_, size_t buf_len_ )
{
  buf=buf_;
  buf_len=buf_len_;
  buf_pos=0;
}


bool jdk_linesplit::scan_for_line( 
  jdk_string *line_buf, 
  const char *ign, 
  const char *term 
  )
{
  while( buf_pos<buf_len )
  {
    char c = buf[buf_pos++];
    
    if( strchr( ign, c ) )
      continue;
    
    if( strchr( term, c) )
    {
      // got line termination character!
      
      line[line_pos]='\0';
      
      if( line_buf )
      {
        
        // copy line to caller's buffer
        
        line_buf->cpy( line );
      }
      
      line_pos=0;
      // return true
      return true;
    }
    
    
    // put character into line buffer
    
    line[line_pos++]=c;
    if( line_pos>=line_sz-1 )
    {
      // overflow!
      line_pos=0;
      line[line_pos]='\0';
      continue;
    }
    
    
  }
  line[line_pos]='\0';
  return false;
}


