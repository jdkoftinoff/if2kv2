#include "jdk_world.h"

#include "jdk_thread.h"
#include "jdk_log.h"

#undef JDK_HAS_SAFE_MEMORY
#define JDK_HAS_SAFE_MEMORY 1

#ifndef JDK_DEBUG_MEMORY
#define JDK_DEBUG_MEMORY 0
#endif


#ifndef JDK_HAS_SAFE_MEMORY
#if JDK_HAS_THREADS
static jdk_mutex memory_mutex;

void *operator new(size_t sz)
{
  jdk_synchronized(memory_mutex);
  void *p = malloc(sz);
#if JDK_DEBUG_MEMORY	
  if( sz > 1023 )
  {
    jdk_log_debug4( "MEM ALLOC: size %8ld, ptr=0x%08lx", sz, (uint32)p );
  }
#endif	
  if( !p )
  {
    JDK_THROW_ERROR("memory full","");
  }		
  return p;
}

void operator delete( void *p )
{
  jdk_synchronized(memory_mutex);
#if JDK_DEBUG_MEMORY		
  jdk_log_debug4( "MEM FREE []: ptr 0x%08lx", (uint32)p );
#endif	
  free(p);
}

void *operator new[]( size_t sz )
{
  jdk_synchronized(memory_mutex);
  void *p = malloc(sz);
#if JDK_DEBUG_MEMORY	
  
  if( sz > 1023 )
  {
    jdk_log_debug4( "MEM ALLOC []: size %8ld, ptr=0x%08lx", sz, (uint32)p );
  }
#endif
  
  if( !p )
  {
    JDK_THROW_ERROR("memory full []","");
  }		
  return p;
}

void operator delete[]( void *p )
{
  jdk_synchronized(memory_mutex);
#if JDK_DEBUG_MEMORY		
  jdk_log_debug4( "MEM FREE []: ptr 0x%08lx", (uint32)p );
#endif
  free(p);
}		

#endif
#endif






