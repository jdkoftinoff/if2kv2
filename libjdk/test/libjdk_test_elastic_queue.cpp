#include "jdk_world.h"
#include "jdk_log.h"
#include "jdk_settings.h"
#include "jdk_elastic_queue.h"

int main( int argc, char **argv )
{
  jdk_settings_text cmdline(0,argc,argv,0);
  jdk_log_setup( cmdline );

  jdk_elastic_queue<char> q(16);

  while(1)
  {
    char buf[1024];
    int len;

    len = fread( buf, 1, 4, stdin );
    if( len>0 )
    {
      if( q.can_put( len ) )
      {
        jdk_log( JDK_LOG_DEBUG1, "putting %d bytes", len );
        q.put( buf, len );
      }

      if( q.can_get() )
      {
        char c = q.peek();
        q.skip();
        fputc( c, stdout );
      }
    }
    
    if( len!=4 )
    {
      while( q.can_get() )
      {
        char c=q.peek();
        q.skip();
        fputc( c, stdout );
      }
      break;
    }
  }
  return 0;
}
