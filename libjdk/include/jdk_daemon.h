#ifndef _JDK_DAEMON_H
# define _JDK_DAEMON_H

#include "jdk_queue.h"
#include "jdk_pair.h"
#include "jdk_settings.h"

#if JDK_IS_UNIX
#include <pwd.h>
#endif

inline void jdk_daemon_drop_root( const jdk_settings &settings )
{
#if JDK_IS_UNIX
	// set uid/gid is done only on initial execution, not after SIGHUP
	bool set_uid_gid = settings.get_long("setuid");

	if( set_uid_gid )
	{
		uid_t uid;
		uid_t gid;
		
		struct passwd *pw = getpwnam(settings.get("uid").c_str());
		if( pw )
		{			
			uid = pw->pw_uid;
			gid = pw->pw_gid;
		}		
		else
		{
			uid = settings.get_long("uid");
			pw = getpwuid( uid );
			if( pw )
			  gid=pw->pw_gid;
		}				
	
		if( setegid(gid)<0 || seteuid(uid)<0 )
		{
			jdk_log( JDK_LOG_ERROR, "Unable to set UID=%d GID=%d", uid, gid  );
			exit(1);
		}

    	jdk_log( JDK_LOG_NOTICE, "UID=%d GID=%d", geteuid(), getegid() );		
	}
#endif	
}


#if JDK_HAS_FORK
struct jdk_daemon_child_info
{
	jdk_daemon_child_info()
		: pid( -1 ), status( 0 )
	{
	}
	explicit jdk_daemon_child_info( pid_t pid_, int status_ )
		: pid( pid_ ), status( status_ )
	{
	}
	
	pid_t pid;
	int status;
};

extern jdk_queue< jdk_daemon_child_info > *jdk_daemon_child_queue;

extern void jdk_sigterm_handler(int);
extern void jdk_sighup_handler(int);
extern void jdk_sigalarm_handler(int);
extern void jdk_sigchld_handler(int);
#endif

int jdk_daemonize(
				  bool real_daemon,
				  const char *daemon_identity,
				  const char *home_dir,
				  const char *pid_file_dir
				  );

void jdk_daemon_end();
#if JDK_HAS_FORK
pid_t jdk_daemon_fork();
#endif

extern volatile unsigned char jdk_sighup_caught;
extern volatile unsigned char jdk_sigterm_caught;
extern volatile unsigned char jdk_sigalarm_caught;

#endif
