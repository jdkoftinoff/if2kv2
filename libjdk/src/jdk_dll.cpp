
#include "jdk_world.h"

#if JDK_IS_MACOSX

#else

#if JDK_IS_UNIX || JDK_IS_WINE
#include <dlfcn.h>
#elif JDK_IS_WIN32
#endif

#include "jdk_dll.h"



jdk_dll_manager::jdk_dll_manager( const char *fname )
{
  // Try to open the library now and get any error message.
#if JDK_IS_WIN32 && !JDK_IS_WINE
  h=LoadLibraryA( fname );
  if( h==0 )
  {
    jdk_sprintf( err, "%08x", (unsigned int)GetLastError() );
  }
  else
  {
    *err=0;	
  }
  
#elif JDK_IS_UNIX || JDK_IS_WINE
  h=dlopen( fname, RTLD_NOW );
  err=dlerror();
#else 
#error specify platform
#endif	
}

jdk_dll_manager::~jdk_dll_manager()
{
#if JDK_IS_WIN32 && !JDK_IS_WINE
  if( h )
  {
    FreeLibrary(h);
  }
#elif JDK_IS_UNIX	|| JDK_IS_WINE
  // close the library if it isn't null	
  if( h!=0 )
    dlclose(h);
#else
#error specify platform
#endif
}


bool jdk_dll_manager::get_symbol( 
  void **v,
  const char *sym_name
  )
{
  // try extract a symbol from the library
  // get any error message is there is any
  
#if JDK_IS_WIN32 && !JDK_IS_WINE
  if( h!=0 )
  {
    *v = (void *)GetProcAddress( h, sym_name );
    
    if( *v==0 )
    {
      jdk_sprintf( err, "%08x", (unsigned int)GetLastError() );
    }
    else
    {
      *err=0;	
    }
    
    if( *v!=0 )
      return true;
    else
      return false;
  }
  else
  {	
    return false;
  }
#elif JDK_IS_UNIX || JDK_IS_WINE
  if( h!=0 )
  {
    *v = dlsym( h, sym_name );
    err=dlerror();
    if( err==0 )
      return true;
    else
      return false;
  }
  else
  {	
    return false;
  }
#else
#error specify platform
#endif	
}


jdk_dll_factory_base::jdk_dll_factory_base(
  const char *fname,
  const char *factory
  ) : jdk_dll_manager(fname)
{
  // try get the factory function if there is no error yet
  
  factory_func=0;
  
  if( *last_error()==0 )
  {		
    get_symbol( (void **)&factory_func, factory ? factory : "factory0" );
  }
  
}


jdk_dll_factory_base::~jdk_dll_factory_base()
{
}

#endif



