#include "jdk_world.h"
#include "jdk_string.h"
#include "jdk_procmap.h"

extern "C" bool proc1_echo( const jdk_dynbuf &in, jdk_dynbuf &out )
{
  out = in;
  return true;
}

extern "C" bool proc1_print( const jdk_dynbuf &in, jdk_dynbuf &out )
{
  in.extract_stream( stdout );
  out=in;
  out.append_string( "this is 'print'\n" );
  return true;	
}

extern "C" bool proc1_testit( const jdk_dynbuf &in, jdk_dynbuf &out )
{
  out=in;
  out.append_string( "testit\n" );
  return true;	
}


int main( int argc, char **argv )
{
  jdk_str<256> fname;
  fname.cpy( "libjdk_testprocmap.procmap" );
  jdk_procmap<jdk_dynbuf,jdk_dynbuf> procs( fname );
  
  if( argc>1 )
  {		
    jdk_dynbuf in;
    in.append_string( "hello there" );
    //in.append_stream(stdin);
    jdk_dynbuf out;
    jdk_str<128> proc;
    
    while( *(++argv) )
    {			
      proc.cpy(*argv);
      
      fprintf( stdout, "calling %s:\n", proc.c_str() );		
      int r =procs.call_function( 1, proc, in, out );
      if( r==-1 )
      {
        fprintf( stdout, "error\n" );
      }			
      else
      {				
        fprintf( stdout, "result:\n" );
        out.extract_stream( stdout );
      }
      
      in.clear();
      in.append_buf( out );
      out.clear();
    }
    
  }
  return 0;
}

