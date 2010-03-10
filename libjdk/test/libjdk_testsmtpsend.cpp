#include "jdk_world.h"
#include "jdk_smtp_send.h"
#include "jdk_dynbuf.h"

int main( int argc, char **argv )
{
  if( argc<5 )
  {
    fprintf( stderr, "usage:\n\tlibjdk_testsmtpsend SMTPSERVER FROM TO SUBJECT\n" );
    return 1;
  }
  
  jdk_dynbuf buf;
  buf.append_stream( stdin );
  buf.extract_stream( stdout );
  if( jdk_smtp_send( 
        jdk_str<1024>(argv[1]), 
        jdk_str<1024>(argv[2]), 
        jdk_str<1024>(argv[3]), 
        jdk_str<1024>(argv[4]), 
        jdk_str<1024>(argv[5]),
        buf ) )
  {
    fprintf( stderr, "successful\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "failed\n" );
    return 1;
  }		
}

