#include "jdk_world.h"
#include "jdk_mmap_buf.h"
#include "jdk_log.h"

#if JDK_IS_WIN32

jdk_mmap_buf::jdk_mmap_buf( const jdk_string_filename &filename, bool writable, bool shared )
{
  data = (unsigned char *)0;
  data_len = 0;
  buf_len = 0;
  mmap_pos = 0;
  fileHandle = INVALID_HANDLE_VALUE;
  mapFileHandle = INVALID_HANDLE_VALUE;

  fileHandle = CreateFileA( 
    filename.c_str(), 
    writable ? (GENERIC_READ | GENERIC_WRITE) : (GENERIC_READ),
    shared ? (FILE_SHARE_READ | FILE_SHARE_WRITE) : 0,
    0,
    OPEN_EXISTING,
    0,
    0 
    );

 
  if( fileHandle!=INVALID_HANDLE_VALUE )
  {
    mapFileHandle = CreateFileMappingA( 
      fileHandle, 
      0, 
      writable ? PAGE_READWRITE : PAGE_READONLY, 
      0, 
      0, 
      filename.c_str() 
      );

    if( mapFileHandle >0 )
    {
      mmap_pos = MapViewOfFile( 
        mapFileHandle, 
        writable ? (FILE_MAP_READ | FILE_MAP_WRITE) : FILE_MAP_READ, 
        0,
        0,
        0
        );
      mmap_len = GetFileSize( fileHandle, 0 );
    }
  }

  if( mmap_pos!=0 )
  {
    data = (unsigned char *)mmap_pos;
    data_len = mmap_len;
    buf_len = mmap_len;
  }
  else
  {
    // error mapping, so just make a dummy buffer
    data = (unsigned char *)calloc(256,1);
    data_len = 0;
    buf_len = 256;
  }
}
  
jdk_mmap_buf::~jdk_mmap_buf()
{
  if( mmap_pos!=0 )
  {
    UnmapViewOfFile(mmap_pos);
  }
  if( mapFileHandle!=INVALID_HANDLE_VALUE )
  {
    CloseHandle(mapFileHandle);
  }
  if( fileHandle!=INVALID_HANDLE_VALUE )
  {
    CloseHandle(fileHandle);
  }
  
  if( data!=0 && mmap_pos==0 )
  {
    free(data);
  }
}
#endif

#if JDK_IS_UNIX

#include <sys/mman.h>
#include <sys/stat.h>

jdk_mmap_buf::jdk_mmap_buf( const jdk_string_filename &filename, bool writable, bool shared ) 
: 
  file_handle(-1),
  mmap_pos(0), 
  mmap_len(0)
{
  data = (unsigned char *)0;
  data_len = 0;
  buf_len = 0;
  mmap_pos = 0;

//  jdk_log( JDK_LOG_DEBUG1, "MMAP file: %s", filename.c_str() );
  file_handle = open( filename.c_str(), ((writable && shared) ? O_RDWR : O_RDONLY) );
  if( file_handle<0 )
  {
//    jdk_log( JDK_LOG_DEBUG1, "MMAP no file: %s", filename.c_str() );
    ;
  }
  else
  {
    struct stat sbuf;

    if( stat(filename.c_str(),&sbuf) == 0 )
    {      
      mmap_len = sbuf.st_size;
      mmap_pos = mmap( 0, mmap_len, PROT_READ | (writable ? PROT_WRITE : 0), (shared ? MAP_SHARED : MAP_PRIVATE), file_handle, 0 );

//      jdk_log( JDK_LOG_DEBUG4, "MMAP pos=0x%08lx", (unsigned long)mmap_pos );
      if( mmap_pos != (caddr_t)(-1) && mmap_pos!=0 )
      {
        data = (unsigned char *)mmap_pos;
        data_len = mmap_len;
        buf_len = mmap_len;
      }
      else
      {
        close( file_handle );
      }
    }
    else
    {
      close(file_handle);
    }
  }
  if( !data )
  {
    // error mapping, so just make a dummy buffer
    data = (unsigned char *)calloc(256,1);
    data_len = 0;
    buf_len = 256;
//    jdk_log( JDK_LOG_DEBUG1, "MMAP dummy file: %s, data=0x%08lx,", filename.c_str(), data );
  }
}
  
jdk_mmap_buf::~jdk_mmap_buf()
{
  if( mmap_pos!=0 && mmap_pos!=(caddr_t)(-1) )
  {
    munmap( mmap_pos, mmap_len );
    close( file_handle );
  }
  else if( data )
  {
    free(data);
  }
}


#endif
