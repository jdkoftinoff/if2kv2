#ifndef _JDK_SERVER_FAMILY_H
#define _JDK_SERVER_FAMILY_H

#include "jdk_daemon.h"
#include "jdk_log.h"
#include "jdk_settings.h"

#ifndef REAL_SERVER
#define REAL_SERVER JDK_HAS_FORK
#endif

#ifndef JDK_SERVER_FAMILY_MAX_FORKS
#define JDK_SERVER_FAMILY_MAX_FORKS 1000
#endif

#if REAL_SERVER
inline void jdk_server_family_drop_root( const jdk_settings &settings )
{
	if( getuid==0 )
	{
		// set uid/gid is done only on initial execution, not after SIGHUP
		bool set_uid_gid = settings.get_long("setuid");
		
		if( set_uid_gid )
		{
			uid_t uid = settings.get_long("uid");
			uid_t gid = settings.get_long("gid");
			
			if( setegid(gid)<0 || seteuid(uid)<0 )
			{
				jdk_log( JDK_LOG_ERROR, "Unable to set UID=%d GID=%d", uid, gid  );
				exit(1);
			}
			
			jdk_log( JDK_LOG_NOTICE, "UID=%d GID=%d", geteuid(), getegid() );		
		}
	}
}
#endif


class jdk_server_child
{
public:
    virtual ~jdk_server_child() 
	 {
	 }

    virtual void update() = 0;
	virtual bool run() = 0; // return false to exit task. return true to request call again
   
};

#if REAL_SERVER
struct jdk_server_child_info
{
private:
  explicit jdk_server_child_info( const jdk_server_child_info & );
  const jdk_server_child_info & operator = ( const jdk_server_child_info & );
public:

	jdk_server_child_info()
    : 
    pid(-1),
    ended(false),
    start(false),
    child(0)
	{
	}

	void clear()
	{
		pid=-1;	   
		child=0;
	    ended=false;
		start=false;
	}	
   
	pid_t pid;
	bool ended;
	bool start;
	jdk_server_child *child;
};

class jdk_server_family
{
  explicit jdk_server_family( const jdk_server_family & );
  const jdk_server_family & operator = ( const jdk_server_family & );
public:
	jdk_server_family()
	  : num_unique_children(0), parent_pid()
	{
	}
   
    ~jdk_server_family()
	{
		if( count_running_children()>0 )
		{
			term_children();
			sleep(2);
			kill_children();
		}
	}

    void clear()
	{
	   for( int i=0; i<JDK_SERVER_FAMILY_MAX_FORKS; ++i )
	   {
		  children[i].clear();
	   }	   
	   
	}
   

   	bool run( unsigned long max_time_in_seconds=0 )
	{
		parent_pid = getpid();
	    cleanup(); // remove any info on children that exited properly and are not to be restarted
		alarm( max_time_in_seconds ); // wake us up when we need to restart all children
		
		while( true )
		{		
			if( !start_children() ) // try start the children
			{
				return false; // fatal error starting children
			}

			pause(); // wait for some sort of signal
			
			// check to see if any children exited

			while( jdk_daemon_child_queue->can_get() )
			{
				jdk_daemon_child_info exit_info = jdk_daemon_child_queue->peek();
				jdk_daemon_child_queue->skip();
				
				child_exited( exit_info.pid, exit_info.status );
			}
		  
			
			if( jdk_sigterm_caught ) // we were asked to quit nicely
			{
				jdk_log( JDK_LOG_INFO, "SIGTERM" );
				term_children(); // tell children to TERM as well.
				alarm(0); // unset any alarms
			    jdk_sigalarm_caught=0; // and the alarm flag		
				return false;
			}		
			if( jdk_sighup_caught ) // we were asked to restart children
			{
				jdk_log( JDK_LOG_INFO, "SIGHUP" );
		   	    term_children(); // tell children to reload
				jdk_sighup_caught=0; // clear sighup flag
			    alarm(0); // unset any alarms
			    jdk_sigalarm_caught=0; // and the alarm flag
				break;
			}
			if( jdk_sigalarm_caught ) // it is time to restart children
			{
				jdk_log( JDK_LOG_INFO, "SIGALRM" );
				jdk_sigalarm_caught=0; // clear alarm flag
			    term_children();	  // tell children to terminate and restart
				break;
			}						
	    }
		return true;
	}
	
	
	bool add_child( jdk_server_child &child, int repeat_count=1)
	{
		unique_children[ num_unique_children++ ] = &child;
		
		for( int i=0; i<JDK_SERVER_FAMILY_MAX_FORKS; ++i )
		{
			jdk_server_child_info *info = &children[i];
			
			if( info->child==0 )
			{
				info->pid = -1;
				info->start=true;
				info->ended=true; // to trigger initial start via start_children
				info->child = &child;
				if( --repeat_count==0 )
				{
					return true;	
				}					
			}
			
		}
		return false;
	}
	
	bool remove_child( pid_t pid )
	{		
		for( int i=0; i<JDK_SERVER_FAMILY_MAX_FORKS; ++i )
		{
			jdk_server_child_info *info = &children[i];
			
			if( info->pid==pid )
			{
				info->clear();
				return true;
			}
		}
		return false;
	}
	
	void cleanup()
	{
		for( int i=0; i<JDK_SERVER_FAMILY_MAX_FORKS; ++i )
		{
			jdk_server_child_info *info = &children[i];
			
			// TODO: this is confusing
			if( info->pid!=-1 && info->ended && !info->start && info->child )
			{
				info->clear();
			}
		}
	}
	

	void child_exited( pid_t pid, int status)
	{
 		bool foundit=false;
		for( int i=0; i<JDK_SERVER_FAMILY_MAX_FORKS; ++i )
		{
			jdk_server_child_info *info = &children[i];
			
			if( info->pid==pid )
			{			
				foundit=true;
				info->ended=true;
			    info->pid=-1;
				jdk_log( JDK_LOG_DEBUG1, "Known child %d exited with status %d", pid, status );
				break;
			}			
		}		
		if( !foundit )
		{
			jdk_log( JDK_LOG_DEBUG1, "Unknown child %d exited with status %d", pid, status );
		}				
	}
   
	
	bool start_children()
	{
		for( int i=0; i<num_unique_children; ++i )
		{
			if( unique_children[i] )
				unique_children[i]->update();	
		}
		
		for( int i=0; i<JDK_SERVER_FAMILY_MAX_FORKS; ++i )
		{
			jdk_server_child_info *info = &children[i];
			
			if( info->ended && info->start && info->child && info->pid==-1 )
			{
				info->ended=false;
			    pid_t pid=jdk_daemon_fork();

			    jdk_server_child *child = info->child;

				if( pid==0 )
				{
					// in child
					jdk_log( JDK_LOG_INFO, "Child %d started", getpid() );
				    clear();

				    alarm(0);

					jdk_sighup_caught=0;
					jdk_sigalarm_caught=0;

					do
					{
						if( !child->run() )
						{
							break;
						}
						if( jdk_sighup_caught )
						{
							child->update();
							jdk_sighup_caught=0;
						}
					} while( jdk_sigterm_caught==0 );

					return false; // child exiting
				}
				
				if( pid>0 )
				{
					// in parent
					info->pid=pid;					
					jdk_log( JDK_LOG_DEBUG1, "Child %d starting", info->pid );
				}
				
				if( pid<0 )
				{
					// error forking
					jdk_log( JDK_LOG_ERROR, "Unable to fork - stopping everything" );
					kill_children();
					return false;
				}				
			}
			
		}		
 	    return true;
	}	
	
	void kill_children()
	{
		for( int i=0; i<JDK_SERVER_FAMILY_MAX_FORKS; ++i )
		{
			jdk_server_child_info *info = &children[i];
			
	 	    pid_t pid = info->pid;
			if( pid>0 && info->child && !info->ended )
			{
				jdk_log( JDK_LOG_DEBUG1, "KILL to %d", pid );			   
				kill( pid, SIGKILL );
			}
		}			
	}	
   
   	void term_children()
	{
		for( int i=0; i<JDK_SERVER_FAMILY_MAX_FORKS; ++i )
		{
			jdk_server_child_info *info = &children[i];
			
	 	    pid_t pid = info->pid;
			if( pid>0 && info->child && !info->ended )
			{
				jdk_log( JDK_LOG_DEBUG1, "TERM to %d", pid );
				kill( pid, SIGTERM );
			}
		}			
	}

	void hup_parent()
	{
		jdk_log( JDK_LOG_DEBUG1, "HUP to parent %d", parent_pid );
		kill( parent_pid, SIGHUP );
	}
	
	void hup_children()
	{
		for( int i=0; i<JDK_SERVER_FAMILY_MAX_FORKS; ++i )
		{
			jdk_server_child_info *info = &children[i];
		    pid_t pid = info->pid;
			
			if( pid>0 && info->child && !info->ended )
			{
				jdk_log( JDK_LOG_DEBUG1, "HUP to %d", pid );
				kill( pid, SIGHUP );
			}
		}			
	}	
   
   int count_running_children()
   {
	   int cnt=0;
	   for( int i=0; i<JDK_SERVER_FAMILY_MAX_FORKS; ++i )
	   {
		   jdk_server_child_info *info = &children[i];
		   pid_t pid = info->pid;
			
		   if( pid>0 && info->child && !info->ended &&info->start)
		   {
			   ++cnt;
		   }
	   }
	   return cnt;
   }
   
private:
	jdk_server_child_info children[JDK_SERVER_FAMILY_MAX_FORKS];
	jdk_server_child *unique_children[JDK_SERVER_FAMILY_MAX_FORKS];
	int num_unique_children;
	pid_t parent_pid;
};

extern jdk_server_family family;


template <class T>
inline int jdk_server_family_main( 
							   const jdk_setting_description *initial_settings,
							   int argc,
							   char **argv
							   )
{
	jdk_settings settings( initial_settings, argc, argv, 0 );

	if( jdk_daemonize(
	                     (bool)settings.get_long("daemon"),
			             settings.get("name").c_str(),
				         settings.get("home").c_str(),
					     settings.get("pid_dir").c_str() ) <0
					     )
	{
		return 1;
	}   

	jdk_log_setup(
				  settings.get_long( "log_type" ),
				  settings.get( "log_file" ),
				  settings.get_long( "log_detail" )
				  );				  

	jdk_server_family_drop_root( settings );	

//	do
//	{
		sleep(2);
	    T child( settings );
		family.add_child( child, settings.get_long("forks") );		
//	} 
	while( family.run( settings.get_long("max_time") ) );
	
	jdk_daemon_end();
	
	return 0;
}

template <class T1,class T2>
inline int jdk_server_family_main(
							   const jdk_setting_description *initial_settings,
							   int argc,
							   char **argv
							   )
{
	jdk_settings settings( initial_settings, argc, argv, 0 );

	if( jdk_daemonize(
	                     (bool)settings.get_long("daemon"),
			             settings.get("name").c_str(),
				         settings.get("home").c_str(),
					     settings.get("pid_dir").c_str() ) <0
					     )
	{
		return 1;
	}   

	jdk_log_setup(
				  settings.get_long( "log_type" ),
				  settings.get( "log_file" ),
				  settings.get_long( "log_detail" )
				  );				  

	jdk_server_family_drop_root( settings );
//	do
//	{
		sleep(2);	
		T1 child1( settings );
		T2 child2( settings );	
		family.add_child( child1, settings.get_long("forks") );
		family.add_child( child2, settings.get_long("forks") );			
//	} 
	while( family.run( settings.get_long("max_time") ) );
	
	jdk_daemon_end();
	
	return 0;
}

#endif


#endif
