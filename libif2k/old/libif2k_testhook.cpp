
#include "jdk_world.h"

#if JDK_IS_WIN32

#define JDK_WINHOOK_STATICS 1
#include "jdk_winver.h"
#include "jdk_winhook.h"
#include "jdk_socket.h"
#include "jdk_httprequest.h"


class my_win_patch_connect : public jdk_win_patch_connect
{
public:
  
protected:
  virtual int my_connect( int a, struct sockaddr_in *b, int c );	
};



int my_win_patch_connect::my_connect( int a, struct sockaddr_in *b, int c )
{
  MessageBeep(0);
  
  if( JDK_WORDSWAP(b->sin_port) == 80 )
  {
    b->sin_addr.S_un.S_un_b.s_b1 = 209;
    b->sin_addr.S_un.S_un_b.s_b2 = 82;
    b->sin_addr.S_un.S_un_b.s_b3 = 27;
    b->sin_addr.S_un.S_un_b.s_b4 = 70; 		
    b->sin_port = JDK_WORDSWAP(7999);
  }
  
  return call_original( a, b, c );
}




bool do_test(const char *remote_addr )
{
  jdk_inet_client_socket s;
  if( s.make_connection("209.82.27.70",80,0,false ))
  {
    char buf[128];
    if( s.get_remote_addr( buf, 127 ) )
    {
      printf("// connected to: %s\n", buf );
      return true;
    }
  }
  else
  {
    printf("// Failed connection\n" );
  }
  return false;
}

int main()
{
  do_test( "209.82.27.70:80" );
  my_win_patch_connect patcher;
  do_test( "209.82.27.70:80" );	
  return 0;
}
#else

int main()
{
  return 0;
}


#endif
