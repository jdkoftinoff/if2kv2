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



#if JDK_IS_UNIX_THREADS

#define SIGHANDLER_PARAM(a) int a


/* This waits for all children, so that they don't become zombies. */
static void sig_chld(SIGHANDLER_PARAM(signal_type)) 
{
  int pid;
  int status;
  
  while ( (pid = wait3(&status, WNOHANG, NULL)) > 0);
}



static void handle_child(void) 
{
  static bool handled=false;
  
  if( !handled )
  {
    
    struct sigaction act;
    
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = sig_chld;
    sigaction(SIGCHLD, &act, NULL);
    handled=true;
  }
  
}



#endif

jdk_thread::jdk_thread()
:
  please_stop(false),
  my_id(0), 
  stksize(0),
  done(false),
  started(false),
  suicide(false),
#if JDK_HAS_THREADS
  usefork(false)
#else
  usefork(true)
#endif
#if JDK_IS_UNIX_THREADS
  ,
  attr(),
  thr()
#endif
{	
#if JDK_IS_UNIX_THREADS
  handle_child();
#endif	
}



jdk_thread::~jdk_thread() 
{
#if JDK_IS_UNIX_THREADS && JDK_HAS_THREADS
  pthread_attr_destroy( &attr );
#endif
}



bool jdk_thread::run() 
{
  int ret=0;
  
  // If we already started, return false
  
  if( started )
    return false;
  
  // Do any inherited PreRun tasks. Return false if they fail
  
  if( !thread_prerun() )
  {
    return false;	
  }
  
  // start the thread
#if JDK_HAS_FORK
  
  if( usefork )
  {
    ret = fork();
    
    if( ret<0 )
    {
      // in parent, fork failed
      if( suicide )
      {
        delete this;
      }
      
      return false;
    }
    
    if( ret==0 )
    {
      // in forked process
      forked_start();
      _exit(0);
    }
    else
    {
      // in parent, fork worked.
      my_id = ret;
      // todo: delete if suicide?
      return true;
    }
  }
#endif
  
  
#if JDK_HAS_THREADS
# if JDK_IS_UNIX_THREADS
  {
//	    pthread_attr_init(&attr);
//	    pthread_attr_setstacksize(&attr,2*1024*1024);
    ret=::pthread_create( &thr, 0, threaded_start, this );
    if( ret==0 )
    {
      ::pthread_detach( thr );		    
    }
    else
    {
      jdk_log( JDK_LOG_ERROR,"Error creating thread" );		
    }
  }
# elif JDK_IS_BEOS
  
  my_id = spawn_thread(
    threaded_start,
    "jdk_thread",
    B_NORMAL_PRIORITY,
    this
    );
  
  if( my_id==B_NO_MORE_THREADS || id==B_NO_MEMORY )
  {
    ret =-1;
  }
  else
  {
    ret=0;
    resume_thread( id );
  }
  
# elif JDK_IS_WIN32_THREADS && !JDK_FAKE_THREAD
  
  {
    
    my_id = _beginthread(
      threaded_start,             // Thread starting address
      256*1024,                   // Thread stack size
      (void *)this              // Thread start argument
      );
    
    ret = my_id > 0 ? 1 : -1;
  }
  
# endif
#endif
  
  if( ret<0 )
  {
    // the thread was not created.  If we are to commit suicide, then do so.
    if( suicide )
    {
      delete this;
    }
    return false;
  }
  else
  {
    return true;
  }
  
}



bool jdk_thread::blockrun() 
{
  if( !thread_prerun() )
  {
    return false;	
  }
  
  // Do the premain() stuff
  premain();
  
  // Run the main() thread routine
  main();
  
  // Finish by running the postmain() routine
  postmain();
  
  // we are done
  done=true;
  
  // if we are to commit suicide, then do so!
  
  if( suicide )
  {
    delete this;
  }				
  
  return true;
} 




bool jdk_thread::thread_prerun() 
{
  return true;
}



void jdk_thread::premain() 
{	
}



void jdk_thread::postmain() 
{	
}



void jdk_thread::thread_exit() 
{
  postmain();
  done=true;
#if JDK_IS_UNIX_THREADS
  if( usefork )
  {
    if( suicide )
      delete this;
    _exit(0);
  }
  else
  {
    if( suicide )
      delete this;
    
#if JDK_HAS_THREADS
    pthread_exit(0);
#endif
  }
  
#elif JDK_IS_BEOS
  
  ::exit_thread(0);
#elif JDK_IS_WIN32_THREADS && !JDK_FAKE_THREAD
  
  _endthread();
#elif JDK_FAKE_THREAD
  // do nothing
#else
# error
#endif
}



void jdk_thread::sleep( int s ) 
{
#if JDK_IS_UNIX_THREADS
  ::sleep(s);
  
#elif JDK_IS_BEOS
  snooze(s*1000000);
  
#elif JDK_IS_WIN32_THREADS
  Sleep(s*1000);
#else
# error
#endif	
} 

void jdk_thread::sleepms( int ms ) 
{
#if JDK_IS_UNIX_THREADS
  struct timeval t;
  
  t.tv_sec = ms/1000;
  t.tv_usec = (ms*1000)%1000;
  
  select( 0, NULL, NULL, NULL, &t );
#elif JDK_IS_BEOS
  
  snooze( ms*1000 );
  
#elif JDK_IS_WIN32_THREADS
  
  Sleep(ms);
  
#else
  
# error
#endif	
} 

#if JDK_HAS_THREADS && !JDK_FAKE_THREAD
JDK_THREAD_RETVAL jdk_thread::threaded_start( void *self_ ) 
{
  jdk_thread *self = (jdk_thread *)self_;
  
  if( self_ == 0 )
  {
    JDK_THROW_ERROR( "Trying to start null thread", "" );
  }
  
  // Set up the local flags
  
#if JDK_IS_UNIX_THREADS
//	PosixSockInit ignore_sigpipe;
  self->my_id = (jdk_thrid_t)::pthread_self();
#if !JDK_IS_CYGWIN && !JDK_IS_MACOSX
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);	
#endif
  
#elif JDK_IS_BEOS
  // id is already set by run()
#elif JDK_IS_WIN32_THREADS
  self->my_id = (jdk_thrid_t)GetCurrentThread();
#else
# error
#endif
  
  self->done=false;	
  self->started=true;
  
  
  // Do the premain() stuff
  self->premain();
  
  // Run the main() thread routine
  self->main();
  
  // Finish by running the postmain() routine
  self->postmain();
  
  // we are done
  self->done=true;
  
  // if we are to commit suicide, then do so!
  
  if( self->suicide )
  {
    delete self;
  }				
  
#if JDK_IS_UNIX_THREADS
  ::pthread_exit(0);	
  
  return 0;
#elif JDK_IS_BEOS
  ::exit_thread(0);
#elif JDK_IS_WIN32_THREADS
  return;
#else
# error
#endif
  
}


#endif

#if JDK_HAS_FORK
void jdk_thread::forked_start() 
{
  // Set up the local flags
  
  my_id = ::getpid();
  
  done=false;	
  started=true;
  
  
  // Do the premain() stuff
  premain();
  
  // Run the main() thread routine
  main();
  
  // Finish by running the postmain() routine
  postmain();
  
  // we are done
  done=true;
  
  // if we are to commit suicide, then do so!
  
  if( suicide )
  {
    delete this;
  }				
  
  ::_exit(0);
}


#endif





