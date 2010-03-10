#include "jdk_world.h"
#include "jdk_shmem.h"

#if JDK_IS_WIN32 && !JDK_IS_WINE

jdk_shmem::jdk_shmem( const char *mapname, size_t sz )
{
  map = CreateFileMappingA( 
    HANDLE(-1), // use paging file
    NULL, // no security attr
    PAGE_READWRITE, // read/write access
    0,  // high 32 bits of size
    DWORD(sz), // low 32 bits of size
    mapname // name of map object
    );
  
  if( map== NULL )
  {
    mem=0;		
  }
  else
  {
    bool init = ( GetLastError() != ERROR_ALREADY_EXISTS );
    
    mem = (void *)MapViewOfFile( map,  FILE_MAP_WRITE, 0, 0, 0 );	
    
    
    if( init )
    {
      memset(mem, '\0', sz );
    }
    
    
  }
  
  
}


jdk_shmem::~jdk_shmem()
{
  if(mem)			
    UnmapViewOfFile(mem);
  if(map)
    CloseHandle(map);	
}


#endif
