#include "jdk_world.h"
#include "jdk_util.h"

#if JDK_IS_WIN32

int jdk_daemonize(
  bool real_daemon,
  const char *daemon_identity,
  const char *home_dir,
  const char *pid_file_dir
  )
{
  jdk_set_app_name(daemon_identity);
  // do nothing for win32
  return 0;	
}

void jdk_daemon_end()
{	
  
}

void jdk_faemon_fork()
{
}


#else

#if JDK_IS_UNIX
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#if JDK_IS_WIN32
#include <stdlib.h>
#include <stdio.h>
//#include <dir.h>
#endif

#include <fcntl.h>

#include "jdk_string.h"
#include "jdk_log.h"
#include "jdk_daemon.h"

const char *jdk_daemon_identity;
char jdk_pid_file_name[4096]="";

#define JDK_DAEMON_CLOSEALL 0


#if JDK_IS_UNIX
volatile unsigned char jdk_sighup_caught=0;
volatile unsigned char jdk_sigterm_caught=0;
volatile unsigned char jdk_sigalarm_caught=0;
bool jdk_in_fork=false;
jdk_queue< jdk_daemon_child_info > *jdk_daemon_child_queue=0;


void jdk_daemon_kill_child_queue()
{
  delete jdk_daemon_child_queue;
}

pid_t jdk_daemon_fork()
{
  if( !jdk_daemon_child_queue )
  {
    jdk_daemon_child_queue = new jdk_queue< jdk_daemon_child_info >;
    atexit( jdk_daemon_kill_child_queue );
  }
  pid_t id=fork();
  if( id==0 )
  {
    jdk_in_fork = true;
    if( jdk_daemon_child_queue )
      jdk_daemon_child_queue->clear(); // we have no children
    signal( SIGTERM, jdk_sigterm_handler );
    signal( SIGHUP, jdk_sighup_handler );
    signal( SIGALRM, jdk_sigalarm_handler );
    signal( SIGCHLD, jdk_sigchld_handler );			
  }
  return id;
}


void jdk_daemon_end()
{	
  // remove our pid file if we have one
  if( !jdk_in_fork && jdk_pid_file_name[0] )
  {
    unlink( jdk_pid_file_name );
    jdk_pid_file_name[0] = 0;
  }	
}

void jdk_sigterm_handler(int)
{
  signal( SIGTERM, jdk_sigterm_handler );
  
  jdk_sigterm_caught=1;
  
}

void jdk_sighup_handler(int)
{
  signal( SIGHUP, jdk_sighup_handler );
  
  jdk_sighup_caught=1;
}

void jdk_sigalarm_handler(int)
{
  signal( SIGALRM, jdk_sigalarm_handler );
  
  jdk_sigalarm_caught=1;
}

void jdk_sigchld_handler(int)
{
  signal( SIGCHLD, jdk_sigchld_handler );
  int status;
  pid_t pid;
  
  while( (pid= wait3(&status,WNOHANG,NULL))>0 )
  {
    //jdk_log( JDK_LOG_DEBUG1, "pid %d exited status %d", pid, status );		
    if( jdk_daemon_child_queue )
    {
      if( jdk_daemon_child_queue->can_put() )
      {
        jdk_daemon_child_queue->put( jdk_daemon_child_info(pid,status) );
      }
      else
      {	
        //jdk_log( JDK_LOG_ERROR, "Too many children exited" );
        exit(1);
      }
    }
  }		
}

#endif



int jdk_daemonize(
  bool real_daemon,
  const char *daemon_identity,
  const char *home_dir,
  const char *pid_file_dir
  )
{
#if defined(RLIMIT_NOFILE) && JDK_DAEMON_CLOSEALL
  struct rlimit resource_limit =
    {
      0
    };
  int i;
#endif
  
  
  jdk_daemon_identity = daemon_identity;
  
  jdk_set_app_name(daemon_identity);
  // we only fork if we are
  // to be a REAL daemon... a not-real daemon is useful
  // for debuggin purposes
  
#if JDK_IS_UNIX
  int status;
  int fd;
  
  if( real_daemon )
  {
    status = fork();
    
    switch (status)
    {
    case -1:
      perror( "fork()" );
      exit(1);
      break;
    case 0:		// child process
      break;
    default:	// parent process
      exit(0);
      break;
    }
  }
#endif
  
  
#if defined(RLIMIT_NOFILE) && JDK_DAEMON_CLOSEALL
  if( real_daemon )
  {
    resource_limit.rlim_max = 0;
    status = getrlimit( RLIMIT_NOFILE, &resource_limit );
    if( status==-1 )    
    {
      jdk_log( JDK_LOG_ERROR, "getrlimit()" );
      exit(1);
    }
    
    if( resource_limit.rlim_max == 0 )
    {
      jdk_log( JDK_LOG_ERROR, "0 files allowed" );
      exit(1);
    }
    
    // close all open file handles
    for( i=0; i<(int)resource_limit.rlim_max; ++i )
    {
      close( i );
    }
  }
#else
  
  if( real_daemon )
  {
    // close only the std file handles
    for( int i=0; i<3; ++i )
    {
      close( i );
    }
  }
  
#endif
  
#if !__CYGWIN__ && JDK_IS_UNIX && 0	// what is going on here
  status=setsid();
  
  if( status==-1 )
  {
    jdk_log( JDK_LOG_ERROR, "setsid()" );
    exit(1);
  }
#endif
  
#if JDK_IS_UNIX
  if( real_daemon )
  {
    status = fork();
    
    switch (status)
    {
    case -1:
      jdk_log( JDK_LOG_ERROR, "fork()" );
      exit(1);
      break;
    case 0:		// child process
      break;
    default:	// parent process
      exit(0);
      break;
    }
  }
#endif
  
  if( home_dir )
  {
    //char buf[4096];
    //jdk_process_path( home_dir, buf, sizeof(buf) );
    jdk_set_home_dir( home_dir );
#if JDK_IS_UNIX		
    mkdir(home_dir,0750);
#elif JDK_IS_WIN32
    mkdir(home_dir);
#endif
    chdir(home_dir);
  }
  
#if JDK_IS_UNIX
  // set up the file creation mask
  
  umask(0);
  
  // re-open std file handles to point to /dev/null
  
  if( real_daemon )
  {
    fd = open("/dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
  }
#endif
  
#if JDK_IS_UNIX	
  if( pid_file_dir && jdk_daemon_identity && *pid_file_dir )
  {
    char buf[4096];
    
    if( !jdk_formpath(
          buf,
          pid_file_dir,
          jdk_daemon_identity,
          ".pid",
          sizeof( buf )
          ) )
    {
      jdk_log( JDK_LOG_ERROR, "pid file dir overflow" );
      exit(1);
    }
    else
    {
      jdk_process_path( buf, jdk_pid_file_name, sizeof( jdk_pid_file_name ) );
      // store our pid into the pid_file_name
#if defined( O_NOFOLLOW )
      int pid_fd = open( jdk_pid_file_name, O_CREAT | O_TRUNC | O_WRONLY | O_NOFOLLOW, 0640 );
#else
      int pid_fd = open( jdk_pid_file_name, O_CREAT | O_TRUNC | O_WRONLY, 0640 );
#endif
      
      if( pid_fd>=0 )
      {
        char tmpbuf[64];
        sprintf( tmpbuf, "%ld\n", (long)getpid() );
        int len = strlen(tmpbuf);
        if( write( pid_fd, tmpbuf, len)!=len )
        {
          jdk_log( JDK_LOG_ERROR, "Error writing pid file: %s", jdk_pid_file_name );
        }
        
        close( pid_fd );
      }
      else			 
      {
        jdk_log( JDK_LOG_ERROR, "Error creating pid file: %s", jdk_pid_file_name );
      }
    }							   		
    
  }
  
  
  signal( SIGTERM, jdk_sigterm_handler );
  signal( SIGHUP, jdk_sighup_handler );
  signal( SIGALRM, jdk_sigalarm_handler );
  signal( SIGCHLD, jdk_sigchld_handler );	
#endif
  
  return 0;
}

#endif

