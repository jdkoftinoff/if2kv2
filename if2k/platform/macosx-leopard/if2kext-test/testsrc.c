#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <net/if.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/sockio.h>
#include <netinet/in.h>
#include <memory.h>
#include <string.h>

int main( int argc, char **argv )
{
  int sock;
  struct sockaddr_in to;
  struct sockaddr_in from;
  int len = sizeof( struct sockaddr_in );
  struct hostent *r;

  if( argc<4 )
  {
    return 1;
  }
  
  memset( (void *)&to, 0, len );
  memset( (void *)&from, 0, len );

  r = gethostbyname( argv[1] );
  

  if( r )
  {
    int v=1;
    struct ifstat s;

    to.sin_family = r->h_addrtype;
    to.sin_addr.s_addr = *((long*)r->h_addr );
    to.sin_port = strtol( argv[2], 0, 10 );
    sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if( sock<0 )
    {
      perror("socket");
      abort();
    }

    v=strtol( argv[3], 0, 10 );

    from.sin_family = AF_INET;
    from.sin_addr.s_addr = 0x00000001;
    from.sin_port = v;
#if 1
    if( bind( sock, (struct sockaddr*)&from, len ) )
    {
      perror( "1st bind" );
    }

    from.sin_port = 0;
    if( bind( sock, (struct sockaddr*)&from, len ) )
    {
      perror( "2nd bind" );
    }
#endif
    if( connect( sock, (struct sockaddr *)&to, len )>=0 )
    {
      printf( "connected to %s:%u\n", argv[1], ntohs( to.sin_port ) );
      socklen_t l=sizeof(to);
      if( getpeername( sock, (struct sockaddr *)&to, &l ) == 0 )
      {
        printf( "peername is: %lx:%u\n", 
                ntohl( to.sin_addr.s_addr ), 
                ntohs( to.sin_port ));
      }
    }
    else
    {
      printf( "connection failed to %s:%u\n", argv[1], to.sin_port );
    }
    close( sock );
  }

  return 0;
}

