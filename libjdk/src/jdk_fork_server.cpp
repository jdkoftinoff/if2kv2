#include "jdk_world.h"
#include <sys/stat.h>
#include "jdk_fork_server.h"

#if JDK_HAS_FORK

static pid_t fork_child_pids[ JDK_FORK_SERVER_MAX_CHILD ];
static jdk_multi_fork_server * fork_servers[ JDK_FORK_SERVER_MAX_CHILD ];
static int fork_server_count;
static char pid_file_name[4096] = "";
pid_t jdk_fork_server_parent_id = 0;

volatile bool jdk_fork_server_saw_sighup;
volatile bool jdk_fork_server_saw_sigterm;
volatile bool jdk_fork_server_saw_sigalarm;


#define SIGHANDLER_PARAM(a) int a


  void jdk_multi_fork_server::prep_this()
        {
      for( int i=0; i<JDK_FORK_SERVER_MAX_CHILD; ++i )
      {
        children_died[i] = 0;
        children_ids[i] = 0;
      }
			jdk_fork_handle_child();
      if( num_forks > JDK_FORK_SERVER_MAX_CHILD-1 )
      {
        num_forks = JDK_FORK_SERVER_MAX_CHILD-1;
      }
    }


	jdk_multi_fork_server::jdk_multi_fork_server(
    const char *server_name_,
		jdk_fork_server_child &serv_,
		const jdk_string &local_binding, 
    int num_forks_,
    int max_child_requests_,
    int max_child_time_
		)	 
		:
    server_name( server_name_ ),
		serv( serv_ ),
		server_sock( local_binding ),
    num_forks( num_forks_ ),
    max_child_requests( max_child_requests_ ),
    max_child_time( max_child_time_ ),
    stopping(false),
    children_died_count(0),
    children_ids_count(0)
		{
      jdk_log( JDK_LOG_DEBUG1, "Server: %s interface: %s", server_name.c_str(),  local_binding.c_str() );
      jdk_log( JDK_LOG_DEBUG1, "Server: %s max_sockets: %d", server_name.c_str(),  num_forks );
      jdk_log( JDK_LOG_DEBUG1, "Server: %s max_requests: %d", server_name.c_str(),  max_child_requests );
      jdk_log( JDK_LOG_DEBUG1, "Server: %s max_time: %d", server_name.c_str(),  max_child_time );

      prep_this();
		}

	jdk_multi_fork_server::jdk_multi_fork_server( 
		const jdk_settings &settings,
    const char *server_name_, 
		jdk_fork_server_child &serv_
		)	 
		:
    server_name( server_name_ ),
		serv( serv_ ),
		server_sock( settings.get_prefixed(server_name_,".interface") ),
    num_forks( settings.get_prefixed_long(server_name_,".max.sockets") ),
    max_child_requests( settings.get_prefixed_long(server_name_,".max.requests") ),
    max_child_time( settings.get_prefixed_long(server_name_,".max.time") ),
    stopping(false),
    children_died_count(0),
    children_ids_count(0)
		{
      jdk_log( JDK_LOG_DEBUG1, "Server: %s interface: %s", server_name.c_str(),  settings.get_prefixed(server_name_,".interface").c_str() );
      jdk_log( JDK_LOG_DEBUG1, "Server: %s max_sockets: %d", server_name.c_str(),  num_forks );
      jdk_log( JDK_LOG_DEBUG1, "Server: %s max_requests: %d", server_name.c_str(),  max_child_requests );
      jdk_log( JDK_LOG_DEBUG1, "Server: %s max_time: %d", server_name.c_str(),  max_child_time );
      prep_this();
		}


	jdk_multi_fork_server::~jdk_multi_fork_server()
		{
      if( count_running()>0 )
      {
        stop( SIGTERM );
        sleep(2);
        if( count_running()>0 )
        {
          stop( SIGKILL );
        }
      }
		}

  const jdk_string &jdk_multi_fork_server::get_server_name() const
    {
      return server_name;
    }

	bool jdk_multi_fork_server::start()
		{
      stopping = false;
      int count=num_forks;
			for( int i=0; i<count; ++i )
			{
        if( !start_one_child(i) )
        {
          return false;
        }
			}
			return true;
		}

	bool jdk_multi_fork_server::start_one_child(int time_offset)
		{
      stopping = false;

      pid_t pid=fork();
      if( pid==0 )
      {
        // in child
        jdk_log( JDK_LOG_DEBUG2, "%s child started", server_name.c_str() );

        jdk_fork_handle_sighup_sigterm_sigalarm();
        run();
        jdk_log( JDK_LOG_DEBUG2, "%s child %d ending", server_name.c_str(), getpid() );
        _exit(0);
      }
      else if( pid<0 )
      {
        jdk_log( JDK_LOG_ERROR, "Unable to start %s child process", server_name.c_str() );
        return false;
      }
      else if( pid>0 )
      {
        while( children_ids[ children_ids_count ] != 0 )
        {
          children_ids_count++;
          if( children_ids_count==JDK_FORK_SERVER_MAX_CHILD )
          {
            children_ids_count = 0;
          }
        }

        children_ids[ children_ids_count ] = pid;

        if( children_ids_count == JDK_FORK_SERVER_MAX_CHILD )
          children_ids_count=0;
        jdk_fork_register_child( pid, this );
      }			
			return true;
    }

  int jdk_multi_fork_server::count_running()
    {
      int count=0;
      for( int i=0; i<JDK_FORK_SERVER_MAX_CHILD; ++i )
      {
        if( children_ids[i]!=0 )
          count++;
      }
      return count;
    }

	void jdk_multi_fork_server::stop(int sig)
		{
      stopping=true;
      for( int i=0; i<JDK_FORK_SERVER_MAX_CHILD; ++i )
      {
        int pid = children_ids[i];
        if( pid != 0 )
        {
          kill( children_ids[ i ], SIGTERM );
        }
      }
		}

  void jdk_multi_fork_server::notify_sig_chld( pid_t pid )
    {
      if( !stopping )
      {
        int count = children_died_count;
        children_died[count]=pid;

        jdk_log( JDK_LOG_DEBUG4, "Got SIGCHLD for %s child pid %d", server_name.c_str(), pid );

        if( ++count == JDK_FORK_SERVER_MAX_CHILD )
        {
          count = 0;
        }
        children_died_count = count;
      }
    }

  bool jdk_multi_fork_server::restart_dead_children()
    {
      stopping = false;

      // find dead children in list given from notify_sig_chld
			for( int i=0; i<JDK_FORK_SERVER_MAX_CHILD; ++i )
			{
        int pid = children_died[i];
        if( pid )
        {
          // found one. 
          // clear the flag
          jdk_log( JDK_LOG_INFO, "Found child %s %d ended", server_name.c_str(), pid );
          children_died[i] = 0;

          // Now, clear the pid in our running list
          for( int j=0; j<JDK_FORK_SERVER_MAX_CHILD; ++j )
          {
            if( children_ids[j]==pid )
            {
              jdk_log( JDK_LOG_DEBUG2, "Removing child %s %d from children_ids[]", server_name.c_str(), pid );
              children_ids[j] = 0;
              break;
            }
          }

          // try start him again
          if( !start_one_child() )
          {
            jdk_log( JDK_LOG_ERROR, "Failed to restart %s child", server_name.c_str() );
            return false;
          }
        }
			}
			return true;
    }

 

	void jdk_multi_fork_server::run(int time_offset)
		{
      alarm( max_child_time+time_offset );
      int request_count=0;

			while(1)
			{
        if( request_count>=max_child_requests )
        {
          jdk_log( JDK_LOG_INFO, "Child %s Max requested performed. Exiting.", server_name.c_str() );
          break;
        }

        if( jdk_fork_server_saw_sigalarm )
        {
          jdk_log( JDK_LOG_INFO, "SIGALARM: Child %s exit requested", server_name.c_str() );
          break;
        }

        if( jdk_fork_server_saw_sighup )
        {
          jdk_log( JDK_LOG_INFO, "SIGHUP: Child %s exit requested", server_name.c_str() );
          break;
        }

        if( jdk_fork_server_saw_sigterm )
        {
          jdk_log( JDK_LOG_INFO, "SIGTERM: Child %s exit requested", server_name.c_str() );
          break;
        }


				jdk_select_manager sel;
				sel.add_read( server_sock );
				sel.set_timeout( 1, 0 );
				int r = sel.do_select();
				if( r==0 )
				{
          continue;
				}
				else if( r>0 )
				{
          request_count++;
					if( sel.can_read( server_sock ) )
					{
						jdk_inet_client_socket client;
            if( server_sock.accept( &client ) )
						{
							client.setup_socket_blocking(true);
              jdk_log( JDK_LOG_DEBUG1, "Incoming Connection for %s %d", server_name.c_str(), getpid() ); 
							bool repeat=serv.handle_connection( client );
              jdk_log( JDK_LOG_DEBUG1, "Finished Connection for %s %d", server_name.c_str(), getpid() );
              client.close();                    
							if( !repeat )
							{
                jdk_log( JDK_LOG_INFO, "Server requested end of child %s %d", server_name.c_str(), getpid() );
								break;
							}
						}
						else
						{
							jdk_log( JDK_LOG_DEBUG1, "Didn't catch incoming socket in child %s %d", server_name.c_str(), getpid() );
						}
					}
				}
				else if( r<0 )
				{
					// error doing select! Abort.
					jdk_log( JDK_LOG_WARNING, "Error doing select() in child %s %d", server_name.c_str(), getpid() );
					break;
				}
			}
		}



/* This waits for all children, so that they don't become zombies. */
static void sig_chld(SIGHANDLER_PARAM(signal_type)) 
{
  int pid;
  int status;
  
//  while ( (pid = wait3(&status, WNOHANG, NULL)) > 0)
  while ( (pid = waitpid(0,&status, WNOHANG)) > 0)
  {
//    jdk_log( JDK_LOG_INFO, "sig_chld: pid %d", pid );

    for( int i=0; i<JDK_FORK_SERVER_MAX_CHILD; ++i )
    {
      if( fork_child_pids[i] == pid )
      {
        fork_servers[i]->notify_sig_chld( pid );
        fork_child_pids[i] = 0;
        fork_servers[i] = 0;
        break;
      }
    }
  }
}

static void sig_hup(SIGHANDLER_PARAM(signal_type))
{
  jdk_fork_server_saw_sighup = true;
}

static void sig_int(SIGHANDLER_PARAM(signal_type))
{
  jdk_fork_server_saw_sigterm = true;
}

static void sig_alarm(SIGHANDLER_PARAM(signal_type))
{
  jdk_fork_server_saw_sigalarm = true;
}

void jdk_fork_handle_sighup_sigterm_sigalarm()
{
  struct sigaction act;
  
  jdk_fork_server_saw_sighup =false;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = sig_hup;
  sigaction(SIGHUP, &act, NULL);
  
  jdk_fork_server_saw_sigterm =false;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = sig_int;
  sigaction(SIGTERM, &act, NULL);

  jdk_fork_server_saw_sigalarm =false;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = sig_alarm;
  sigaction(SIGALRM, &act, NULL);
}

void jdk_fork_handle_child() 
{
  static bool handled=false;
  
  if( !handled )
  {    
    struct sigaction act;

    for( int i=0; i<JDK_FORK_SERVER_MAX_CHILD; ++i )
    {
      fork_child_pids[i]=0;
      fork_servers[i]=0;
    }
    fork_server_count=0;
    
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = sig_chld;
    sigaction(SIGCHLD, &act, NULL);
    handled=true;
  }
  
}

void jdk_fork_register_child( pid_t pid, jdk_multi_fork_server *self )
{
  int i=fork_server_count;

  while( fork_child_pids[i]!=0 || fork_servers[i]!=0 )
  {
    if( ++i == JDK_FORK_SERVER_MAX_CHILD )
      i=0;    
  }
  fork_server_count=i;
  
  fork_servers[i] = self;
  fork_child_pids[i] = pid;

}

void jdk_fork_server_remove_pid()
{
  unlink( pid_file_name );
}

int jdk_fork_server_daemonize(
  const char *daemon_identity,
  bool real_daemon,
  const char *home_dir,
  const char *pid_file_dir
  )
{  
  jdk_set_app_name(daemon_identity);
  // we only fork if we are
  // to be a REAL daemon... a not-real daemon is useful
  // for debugging purposes
  
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
  
  
  if( real_daemon )
  {
    // close only the std file handles
    for( int i=0; i<3; ++i )
    {
      close( i );
    }
  }
 

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
  
  if( home_dir )
  {
    jdk_set_home_dir( home_dir );

    mkdir(home_dir,0750);

    chdir(home_dir);
  }
  

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

  jdk_fork_server_parent_id = getpid();

  if( pid_file_dir && daemon_identity && *pid_file_dir )
  {
    char buf[4096];
    
    if( !jdk_formpath(
          buf,
          pid_file_dir,
          daemon_identity,
          ".pid",
          sizeof( buf )
          ) )
    {
      jdk_log( JDK_LOG_ERROR, "pid file dir overflow" );
      exit(1);
    }
    else
    {
      jdk_process_path( buf, pid_file_name, sizeof( pid_file_name ) );
      // store our pid into the pid_file_name
#if defined( O_NOFOLLOW )
      int pid_fd = open( pid_file_name, O_CREAT | O_TRUNC | O_WRONLY | O_NOFOLLOW, 0640 );
#else
      int pid_fd = open( pid_file_name, O_CREAT | O_TRUNC | O_WRONLY, 0640 );
#endif
      
      if( pid_fd>=0 )
      {
        char tmpbuf[64];
        sprintf( tmpbuf, "%ld\n", (long)getpid() );
        int len = strlen(tmpbuf);
        if( write( pid_fd, tmpbuf, len)!=len )
        {
          jdk_log( JDK_LOG_ERROR, "Error writing pid file: %s", pid_file_name );
        }
        
        close( pid_fd );
        atexit( jdk_fork_server_remove_pid );
      }
      else			 
      {
        jdk_log( JDK_LOG_ERROR, "Error creating pid file: %s", pid_file_name );
      }
    }							   		
    
  }
    
  return 0;
}


void jdk_fork_server_run_list( 
  jdk_multi_fork_server **servers, 
  jdk_fork_server_child **childs, 
  int servers_count,
  jdk_fork_server_reloader *reloader
  )
{
  int i;
  bool first_time=true;

  if( reloader && reloader->get_reload_time_in_seconds()>60 )
  {
    alarm( reloader->get_reload_time_in_seconds() );
  }

  while( 1 )
  {
    if( first_time )
    {
      for(i=0; i<servers_count; ++i )
      {
        jdk_log( JDK_LOG_INFO, "About to load %s", servers[i]->get_server_name().c_str() );
        childs[i]->load_settings();
        jdk_log( JDK_LOG_INFO, "About to start %s", servers[i]->get_server_name().c_str() );
        servers[i]->start();
      }
      first_time=false;
    }

    sleep(1);

    if( jdk_fork_server_saw_sighup )
    {
      jdk_fork_server_saw_sighup = false;
      jdk_log( JDK_LOG_INFO, "Got SIGHUP: Reloading servers" );

      if( reloader )
      {
        reloader->reload();
      }
      for(i=0; i<servers_count; ++i )
      {
       childs[i]->handle_sighup();
      }

      for(i=0; i<servers_count; ++i )
      {
       servers[i]->stop(SIGHUP);
      }
    }
    
    // check for sigterm to stop server
    if( jdk_fork_server_saw_sigterm )
    {
      jdk_log( JDK_LOG_INFO, "Got SIGTERM: Ending servers" );

      for(i=0; i<servers_count; ++i )
      {
       servers[i]->stop(SIGTERM);
      }

      sleep(1);
      break;
    }
    
    // If any children have died, time to restart them.
    for(i=0; i<servers_count; ++i )
    {
      servers[i]->restart_dead_children();
    }
  }     

  for(i=0; i<servers_count; ++i )
  {
    servers[i]->stop();
  }
}

int jdk_fork_server_signal_via_pidfile( const jdk_string_filename &pid_filename, int sig )
{
  jdk_str<256> pid_text;
  int r=0;

  FILE *f=fopen( pid_filename.c_str(), "rt" );

  if( f && jdk_read_string_line( &pid_text,f) )
  {
    if( f )
    {
      fclose(f);
    }

    pid_t pid = strtol(pid_text.c_str(),0,10);
    if( pid>0 )
    {
      int r = kill(pid,sig);
      if( r==0 )
      {
        r=1;
      }
    }
  }
  else
  {
    if( f )
    {
      fclose(f);
    }
  }
  return r;
}

#endif




