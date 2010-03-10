
#include "jdk_world.h"

#if JDK_IS_WIN32

#include "jdk_socket.h"
#include "jdk_httprequest.h"

/* first 8 bytes of ws2_32.dll's connect() entry point (intel syntax)
   55 = push ebp 
   8b ec = mov ebp, esp 
   83 ec 18 = sub esp, 0x18 
   53 = push ebx 
   56 = push esi 
*/

extern "C" int WINAPI hook_connect(int, void *, int );
extern "C" int WINAPI hook_connect_win98(int, void *, int );
DWORD hook_connect_reentry=0;
DWORD hook_connect_entry=(DWORD)hook_connect;

#if 1
asm(
  "   .text\n"
  "_hook_connect@12:\n"
  "   call _my_connect\n"
  "	pushl %ebp\n"
  "   movl %esp,%ebp\n"
  "   subl $0x18, %esp\n"
  "   pushl %ebx\n"
  "   pushl %esi\n"
  "   movl _hook_connect_reentry,%ebx\n"
  "   jmp *%ebx\n" 
  "   jmp *0x11223344\n"	
  );
asm(
  "   .text\n"
  "_hook_connect_win98@12:\n"
  "   call _my_connect\n"
  "	pushl %ebp\n"
  "   movl %esp,%ebp\n"
  "   subl $0x10, %esp\n"
  "   pushl %ebx\n"
  "   pushl %esi\n"
  "   movl _hook_connect_reentry,%ebx\n"
  "   jmp *%ebx\n" 
  "   jmp *0x11223344\n"	
  );
#endif


extern "C" int my_connect(
  DWORD dummy, 
  int sock_fd, 
  struct sockaddr_in *serv_addr,
  int addrlen 
  )
{
#if 0	
  char str[1024];
  sprintf( str, "sock_fd=%d, addrlen=%d", sock_fd,addrlen);
  MessageBox( 0, str, "MY CONNECT CALLED", MB_OK );
#endif
  
#define VERIFY_POINTER(P,LEN) ((!IsBadReadPtr((void *)(P), (LEN)) && !IsBadWritePtr((void*)(P),(LEN))))
#define VERIFY_POINTER1(P) VERIFY_POINTER((P),4)
  
  if( serv_addr && VERIFY_POINTER1(serv_addr) && serv_addr->sin_family==AF_INET )
  {
#define WORDSWAP(a)		((((a)&0xff)<<8) + (((a)&0xff00)>>8));
    
    int port = WORDSWAP(serv_addr->sin_port);
    
    if( port == 80 )
    {
      serv_addr->sin_addr.S_un.S_un_b.s_b1 = 209;
      serv_addr->sin_addr.S_un.S_un_b.s_b2 = 82;
      serv_addr->sin_addr.S_un.S_un_b.s_b3 = 27;
      serv_addr->sin_addr.S_un.S_un_b.s_b4 = 70; 
      serv_addr->sin_port = WORDSWAP(7070);
    }
    
#if 0		
    sprintf( str, "%d.%d.%d.%d:%d", 
             serv_addr->sin_addr.S_un.S_un_b.s_b1,
             serv_addr->sin_addr.S_un.S_un_b.s_b2,
             serv_addr->sin_addr.S_un.S_un_b.s_b3,
             serv_addr->sin_addr.S_un.S_un_b.s_b4,
             port );
    
    MessageBox( 0, str, "Destination addr", MB_OK );
#endif		
  }
  
  return 0;	
}


int patch_connect()
{
  char str[2048];
  
  HMODULE m = GetModuleHandle( "ws2_32.dll" );
  if( !m )
  {
    MessageBox( 0, "No Module handle for ws2_32.dll", "error", MB_OK );
    return 0;
  }
  
  volatile FARPROC cptr = GetProcAddress( m, "connect" );
  volatile unsigned char *data = (volatile unsigned char *)cptr;
  
  if( !cptr )
  {
    MessageBox( 0, "no address for connect", "error", MB_OK  );
    return 0;
  }
  sprintf( str, "address of connect is 0x%08lx", (DWORD)data );
  MessageBox( 0, str, "info", MB_OK );
  
  DWORD oldProtect;
  if( !VirtualProtect( (void*)cptr, 4096, PAGE_EXECUTE_WRITECOPY, &oldProtect ) )
  {		
    if( !VirtualProtect( (void*)cptr, 4096, PAGE_EXECUTE_READWRITE, &oldProtect ) )
    {
      MessageBox( 0, "VirtualProtect failed", "error", MB_OK );					
      return 0;
    }		
  }
  
  hook_connect_reentry = ((DWORD)(data))+8;
  sprintf( str,"reentry=%08lx", hook_connect_reentry );
  MessageBox(0,str,"INFO",MB_OK );
  
  sprintf( str, "original: %02x %02x %02x %02x %02x %02x %02x %02x",
           data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7] 
    );
  MessageBox( 0, str, "info", MB_OK );
  
  data[0] = 0xff; /* long jump, indirect */
  data[1] = 0x25;
  DWORD *jmpaddr = (DWORD *)(data+2);
  *jmpaddr = (DWORD)&hook_connect_entry;
  
  sprintf( str, "new: %02x %02x %02x %02x %02x %02x %02x %02x",
           data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7] 
    );
  MessageBox( 0, str, "info", MB_OK );
  
  
  /*
    DWORD other;
    if( !VirtualProtect( cptr, 4096, oldProtect, &other ) )
    {
    MessageBox( 0, "SecondVirtualProtect failed", "error", MB_OK );
    return 0;
    }
  */
  
  return 1;	
}


int main()
{
  jdk_inet_client_socket s;
  MessageBox( 0, "hello", "World", MB_OK );
  
  if( patch_connect() )   
  {
    MessageBox( 0, "About to try connect", "INFO", MB_OK );
    
    if( s.make_connection( "209.82.27.70", 80, 0, false ) )
    {
      MessageBox( 0, "Connection made!", "INFO", MB_OK );
      s.close();
      
      jdk_dynbuf b;
      jdk_http_response_header header;
      
      int resp = jdk_http_get( "http://www.jdkoftinoff.com/index.php", &b, 1024*1024, &header, 0, false );
      
      b.extract_file( "url_response.html" );
      char str[1024];
      sprintf( str, "Response code: %d", resp );
      MessageBox( 0, str, "INFO", MB_OK );
      MessageBox( 0, "stored data in url_response.html", "INFO", MB_OK );
    }
    else
    {
      MessageBox( 0, "Cant make connection", "INFO", MB_OK );			
    }
    
  }
  
  
  MessageBox( 0, "DONE", "INFO", MB_OK );
  
  return 0;
}

#else

int main()
{
  return 0;
}

#endif
