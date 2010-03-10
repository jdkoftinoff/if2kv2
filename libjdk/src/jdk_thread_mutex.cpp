#include "jdk_world.h"

#include "jdk_thread.h"
#include "jdk_daemon.h"
#include "jdk_log.h"
#include "jdk_error.h"

#if JDK_IS_WIN32_THREADS
#include <process.h>
#endif


#if JDK_IS_UNIX_THREADS
# include <signal.h>
# include <sys/time.h>
# include <unistd.h>
# include <sys/wait.h>
# if JDK_HAS_THREADS
#  include <pthread.h>
# endif
# include "jdk_socket_unix.h"
#endif



//------------------------------------------------------------------------------------


jdk_mutex::jdk_mutex( const char *name_) : name( name_ )
#if !JDK_IS_WIN32
, my_mutex()
#endif
{
#if JDK_HAS_THREADS   
#if JDK_IS_UNIX_THREADS 
  static pthread_mutex_t dummy_mutex = PTHREAD_MUTEX_INITIALIZER;
  
  my_mutex = dummy_mutex;
  
  pthread_mutex_init( &my_mutex, 0 );
#elif JDK_IS_BEOS
  
  mutex = create_sem(1,"jdk_mutex");
  ben_val = 0;
  //if ((max_sem = create_sem(1, "max_sem")) < B_NO_ERROR) 
  //    return B_ERROR;
#elif JDK_IS_WIN32_THREADS
  InitializeCriticalSection(&crit);
#else
# error
#endif		
#endif
  if( name )
    jdk_log_debug5( "mutex '%s' created", name );
}



jdk_mutex::~jdk_mutex() 
{
#if JDK_HAS_THREADS
#if JDK_IS_UNIX_THREADS 
  pthread_mutex_destroy( &my_mutex );
#elif JDK_IS_BEOS
  delete_sem( my_mutex );
#elif JDK_IS_WIN32_THREADS
  DeleteCriticalSection( &crit );
#else
# error
#endif
#endif
  if( name )
    jdk_log_debug5( "mutex '%s' destroyed", name );
}



void jdk_mutex::get() 
{
#if JDK_HAS_THREADS
#if JDK_IS_UNIX_THREADS 
  if( pthread_mutex_lock( &my_mutex ) < 0 )
  {
    JDK_THROW_ERROR( "Error getting mutex", "" );
  }
  
#elif JDK_IS_BEOS
  
  int32 previous = atomic_add(&ben_val, 1);
  
  if (previous >= 1)
  {
    if (acquire_sem(my_mutex) != B_NO_ERROR)
    {
      previous = atomic_add(&ben_val,-1);
      return;
    }
  }
#elif JDK_IS_WIN32_THREADS
  EnterCriticalSection( &crit );
#else
# error
#endif
#endif
}



bool jdk_mutex::tryget() 
{
#if JDK_HAS_THREADS
#if JDK_IS_UNIX_THREADS
  int r=pthread_mutex_trylock( &my_mutex );
  
  if( r<0 )
  {
    JDK_THROW_ERROR( "Error trying to get mutex", "" );
  }
  
  return r==0;
#elif JDK_IS_BEOS
  
  int32 previous = atomic_add(&ben_val, 1);
  
  if (previous >= 1)
  {
    if (acquire_sem_etc(
          my_mutex,
          1,
          B_TIMEOUT,
          0
          ) != B_NO_ERROR)
    {
      previous = atomic_add(&ben_val,-1);
      return false;
    }
  }
  
#elif JDK_IS_WIN32_THREADS
  return false;
//	return TryEnterCriticalSection( &crit )==0; // doesnt work on win95
#else
# error
#endif
#else
  return true;
#endif
}



void jdk_mutex::release() 
{
#if JDK_HAS_THREADS
#if JDK_IS_UNIX_THREADS
  //int r=
  if( pthread_mutex_unlock( &my_mutex )<0 )
  {
    JDK_THROW_ERROR( "Error releasing mutex", "" );
  }
#elif JDK_IS_BEOS
  int32 previous = atomic_add(&ben_val, -1);
  if (previous > 1)
    release_sem(my_mutex);
#elif JDK_IS_WIN32_THREADS
  LeaveCriticalSection(&crit);
#else
# error
#endif
#endif
}




//------------------------------------------------------------------------------------

jdk_recursivemutex::jdk_recursivemutex( const char *name_ ) 
: jdk_mutex(name_)
#if JDK_HAS_THREADS
,
  owner_id(0),
  owner_count(0),
  guard(),
  owner_pid(0)
#endif
{
}



jdk_recursivemutex::~jdk_recursivemutex() 
{
}



void jdk_recursivemutex::get() 
{
#if JDK_HAS_THREADS
  bool wait_for_it=false;
  bool cur_thread_has_it_already=false;
  
  
  {
    guard.get();
    if( owner_id == JDK_CURTHREADID && owner_count>0 )
    {
      cur_thread_has_it_already=true;		    
    }
    guard.release();
  }
  
  if( cur_thread_has_it_already )
  {
    guard.get();
    if( owner_id == JDK_CURTHREADID && owner_count>0 )
    {
      owner_count++;
    }
    else
    {
      cur_thread_has_it_already=false;
    }
    guard.release();
  }
  
  if( !cur_thread_has_it_already )
  {
    wait_for_it=true;
  }
  
  if( wait_for_it )
  {
    if( name )
      jdk_log_debug5( "[%8lx] mutex tryget '%s'",  (unsigned int)JDK_CURTHREADID, name );
    
    jdk_mutex::get();
    
    guard.get();		
    owner_id = JDK_CURTHREADID;
#if JDK_IS_UNIX_THREADS		
    owner_pid = getpid();
#endif		
    owner_count=1;	
    guard.release();
    
    if( name )
    {
      jdk_log_debug5( "[%8lx] mutex got '%s'", (unsigned int)JDK_CURTHREADID, name );    
    }
  }
  
#endif	
}



bool jdk_recursivemutex::tryget() 
{
#if 0// broken
#if JDK_HAS_THREADS   
  bool wait_for_it=false;
  bool cur_thread_has_it_already=false;
  bool got_it=false;
  
  {
    jdk_synchronized(guard);
    if( owner_id == JDK_CURTHREADID && owner_count>0 )
    {
      cur_thread_has_it_already=true;		    
    }
  }
  
  if( cur_thread_has_it_already )
  {
    jdk_synchronized(guard);
    if( owner_id == JDK_CURTHREADID && owner_count>0 )
    {
      owner_count++;
      got_it=true;
    }
    else
    {
      cur_thread_has_it_already=false;
    }
  }
  
  if( !cur_thread_has_it_already )
  {
    wait_for_it=true;
  }
  
  if( wait_for_it )
  {
    got_it=jdk_mutex::tryget();
    if( got_it )
    {
      owner_id = JDK_CURTHREADID;
#if JDK_IS_UNIX_THREADS		 
      owner_pid = getpid();
#endif					
      owner_count++;			
      
      if( owner_count==1 && name)
      {
        jdk_log_debug5( "[%8lx] mutex got '%s'", (unsigned int)JDK_CURTHREADID, name );    
      }
      
    }
  }
  return got_it;
#else
  return true;
#endif
#else
  return false;
#endif
}



void jdk_recursivemutex::release() 
{
#if JDK_HAS_THREADS   
  
  // if we already own it, just decrease the count and return
  
  if( owner_id == JDK_CURTHREADID && owner_count>0 )
  {
    int previous_count;
    {
      guard.get();
      --owner_count;
      previous_count=owner_count;
      guard.release();
    }
    
    // this was the last one! free it for real
    if( previous_count==0 )
    {	
      if( name )
        jdk_log_debug5( "[%8lx] mutex released '%s'", (unsigned int)JDK_CURTHREADID, name );
      
      jdk_mutex::release();	   
    }	
    
  }
  else
  {
    // this is a logic error - the calling thread did not own the mutex	
    if( name )
      jdk_log_debug5( "[%8lx] mutex error '%s'", (unsigned int)JDK_CURTHREADID, name );    
    JDK_THROW_ERROR( "Recursive mutex logic error", "" );
  }
#endif	
}






