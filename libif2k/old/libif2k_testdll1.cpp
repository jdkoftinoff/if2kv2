
#include "jdk_world.h"

#if JDK_IS_WIN32

#define JDK_WINHOOK_STATICS 1
#include "jdk_winver.h"
#include "jdk_winhook.h"

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

#if 1
void *operator new(size_t sz)
{
  return (void *)LocalAlloc(LPTR,sz);
}

void operator delete( void *p )
{
  LocalFree((HLOCAL)p);
}

void *operator new[]( size_t sz )
{
  return (void *)LocalAlloc(LPTR,sz);
}

void operator delete[]( void *p )
{
  LocalFree((HLOCAL)p);
}		
#endif



my_win_patch_connect *patcher1;

int WinMain(
				   HINSTANCE hInstance_,          	// current instance
				   HINSTANCE hPrevInstance_,        // previous instance
				   LPSTR lpCmdLine_,                // command line
				   int nCmdShow_                    // show-window type (open/icon)
				   )
{
  patcher1 = new my_win_patch_connect;

  MessageBox( NULL, "test", "Test", MB_OK );
  
  delete patcher1;

  return 0;
}

extern "C" BOOL WINAPI DllMain(
  IN HINSTANCE hinstDll,
  IN DWORD fdwReason,
  LPVOID lpvReserved
  )
{
  
  switch (fdwReason) {
    
  case DLL_PROCESS_ATTACH:
    patcher1 = new my_win_patch_connect;
    break;
    
  case DLL_THREAD_ATTACH:
    
    break;
    
  case DLL_THREAD_DETACH:
    
    break;
    
  case DLL_PROCESS_DETACH:
    delete patcher1;
    break;
  }
  
  
  return(TRUE);
}
#else

int main()
{
  return 0;
}


#endif
