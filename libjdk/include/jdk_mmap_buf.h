#ifndef __JDK_MMAP_BUF_H
#define __JDK_MMAP_BUF_H

#include "jdk_string.h"
#include "jdk_buf.h"

class jdk_mmap_buf : public jdk_buf
{
private:
  jdk_mmap_buf(); // not allowed.
  jdk_mmap_buf( const jdk_mmap_buf &other ); // not allowed
  const jdk_mmap_buf & operator = ( const jdk_mmap_buf &other ); // not allowed
public:
  jdk_mmap_buf( const jdk_string_filename &filename, bool writable=false, bool shared=false );
  
  virtual ~jdk_mmap_buf();

private:

#if JDK_IS_UNIX
  int file_handle;
  void *mmap_pos;
  int mmap_len;
#endif

#if JDK_IS_WIN32
  HANDLE fileHandle;
  HANDLE mapFileHandle;
  void *mmap_pos;
  int mmap_len;
#endif
};


#endif


