#ifndef _JDK_FORK_SERVER_H
#define _JDK_FORK_SERVER_H

#include "jdk_socket.h"
#include "jdk_log.h"
#include "jdk_settings.h"
#include "jdk_thread.h"

class jdk_fork_server_child
{
public:
  jdk_fork_server_child() {}
  virtual ~jdk_fork_server_child() {}
  virtual void load_settings() = 0;
  virtual void handle_sighup() = 0;
  virtual bool handle_connection( jdk_client_socket &sock ) = 0;
};

class jdk_fork_server_reloader
{
public:
  jdk_fork_server_reloader() {}
  virtual ~jdk_fork_server_reloader() {}

  virtual void reload() {}
  virtual int get_reload_time_in_seconds() { return 0;}
};

class jdk_multi_fork_server;


#if JDK_HAS_FORK

#ifndef JDK_FORK_SERVER_MAX_CHILD
#define JDK_FORK_SERVER_MAX_CHILD (4096)
#endif

void jdk_fork_handle_child();
void jdk_fork_register_child( pid_t pid, jdk_multi_fork_server *self );
void jdk_fork_handle_sighup_sigterm_sigalarm();

extern volatile bool jdk_fork_server_saw_sighup;
extern volatile bool jdk_fork_server_saw_sigterm;
extern volatile bool jdk_fork_server_saw_sigalarm;

class jdk_multi_fork_server
{
private:
  void prep_this();

 public:
	jdk_multi_fork_server(
      const char *server_name_,
      jdk_fork_server_child &serv_,
      const jdk_string &local_binding, 
      int num_forks_,
      int max_child_requests_,
      int max_child_time_
      );	 

	jdk_multi_fork_server( 
		const jdk_settings &settings,
    const char *server_name_, 
		jdk_fork_server_child &serv_
      );

	~jdk_multi_fork_server();

    const jdk_string &get_server_name() const;
	bool start();

	bool start_one_child(int time_offset=0);

    int count_running();

	void stop(int sig=SIGTERM);

    void notify_sig_chld( pid_t pid );

    bool restart_dead_children();
 

 private:
	void run(int time_offset=0);
  const jdk_str<256> server_name;
	jdk_fork_server_child &serv;
	jdk_inet_server_socket server_sock;

  int num_forks;
  int max_child_requests;
  int max_child_time;


  bool stopping;
  volatile int children_died_count;
  volatile int children_died[ JDK_FORK_SERVER_MAX_CHILD ];
  int children_ids_count;
  int children_ids[ JDK_FORK_SERVER_MAX_CHILD ];
};


int jdk_fork_server_daemonize(
  const char *daemon_identity,
  bool real_daemon,
  const char *home_dir,
  const char *pid_file_dir
  );

inline int jdk_fork_server_daemonize(
  const jdk_settings &settings
  )
{
  int real_daemon=true;
  if( settings.find("daemon")>=0 )
  {
    real_daemon=settings.get_long("daemon");
  }
  
  bool r = jdk_fork_server_daemonize(
    jdk_get_app_name(),
    real_daemon,
    settings.get("home").c_str(),
    settings.get("pid.dir").c_str()
    );

  jdk_fork_handle_sighup_sigterm_sigalarm();
  return r;
}

inline void jdk_fork_server_simple_main( 
  const jdk_string &local_binding, 
  jdk_fork_server_child &server_worker,
  int num_forks,
  int max_child_requests,
  int max_child_time,
  bool real_daemon, 
  const char *daemon_identity, 
  const char *home_dir, 
  const char *pid_file_dir 
  )
{
  jdk_fork_server_daemonize(
    daemon_identity,
    real_daemon,
    home_dir,
    pid_file_dir
    );

  jdk_fork_handle_sighup_sigterm_sigalarm();

  jdk_multi_fork_server multi_server(
    daemon_identity,
    server_worker,
    local_binding,
    num_forks,
    max_child_requests,
    max_child_time
    );

  multi_server.start();

  while( 1 )
  {
    sleep( 2 );

    // check for sighup to reload server_worker's settings

    if( jdk_fork_server_saw_sighup )
    {
      server_worker.handle_sighup();
      jdk_fork_server_saw_sighup = false;
      jdk_log( JDK_LOG_INFO, "Got SIGHUP: Reloading master server %s", daemon_identity );
      multi_server.stop(SIGHUP);
    }

    // check for sigterm to stop server
    if( jdk_fork_server_saw_sigterm )
    {
      jdk_log( JDK_LOG_INFO, "Got SIGTERM: Ending master server %s", daemon_identity );
      break;
    }

    // If any children have died, time to restart them.
    multi_server.restart_dead_children();
  }

  jdk_log( JDK_LOG_INFO, "Ending master server %s", daemon_identity );

  multi_server.stop();
}


void jdk_fork_server_run_list( 
  jdk_multi_fork_server **servers, 
  jdk_fork_server_child **childs, 
  int servers_count,
  jdk_fork_server_reloader *master_reloader=0
);

int jdk_fork_server_signal_via_pidfile( const jdk_string_filename &pid_fname, int sig );

inline int jdk_fork_server_signal_via_pidfile( const jdk_settings &settings, int sig )
{
  // read pid file and kill the running copy with sigterm
  jdk_string_filename pid_filename;
  pid_filename.form_path( settings.get( "pid.dir" ), jdk_get_app_name(), ".pid" );
  
  return jdk_fork_server_signal_via_pidfile( pid_filename, sig );
}

extern pid_t jdk_fork_server_parent_id;

#endif
#endif

















