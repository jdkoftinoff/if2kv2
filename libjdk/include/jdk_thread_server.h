#ifndef _JDK_THREAD_SERVER_H
#define _JDK_THREAD_SERVER_H

#include "jdk_thread.h"
#include "jdk_daemon.h"
#include "jdk_settings.h"
#include "jdk_socket.h"
#include "jdk_http.h"
#include "jdk_httprequest.h"
#include "jdk_util.h"
#include "jdk_remote_buf.h"


#if JDK_IS_UNIX_THREADS
#include <pwd.h>
#endif

inline void jdk_thread_server_drop_root( const jdk_settings &settings )
{
#if JDK_IS_UNIX_THREADS
	// set uid/gid is done only on initial execution, not after SIGHUP
	bool set_uid_gid = settings.get_long("setuid");

	if( set_uid_gid )
	{
		uid_t uid;
		uid_t gid=getgid();
		
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


class jdk_thread_server_base
{
public:
    jdk_thread_server_base()
    {
    }

    virtual ~jdk_thread_server_base()
    {
    }

    virtual bool run() = 0;

    virtual bool update() = 0;

	virtual void poll() = 0;

    virtual void stop() = 0;

    virtual void kill() = 0;
};


class jdk_thread_server_factory : public jdk_thread_factory
{
public:
    jdk_thread_server_factory()
	{
	}

	virtual ~jdk_thread_server_factory()
	{
	}

	virtual bool run() = 0;

    virtual bool update() = 0;

	virtual void poll() {}
    
	virtual jdk_thread *create_thread( jdk_thread_shared &shared, int id ) = 0;

	virtual jdk_thread_pool_base *create_thread_pool() = 0;
	
    virtual jdk_thread_shared *create_thread_shared() = 0;

    virtual jdk_thread_server_base *create_thread_server() = 0;

    virtual jdk_server_socket &get_server_socket() = 0;
};


class jdk_thread_server : public jdk_thread_server_base
{
public:
    jdk_thread_server( jdk_thread_server_factory &factory_ )
        : factory( factory_ ),
	      pool( (jdk_thread_pool *)(factory.create_thread_pool()) )
    {
    }

    ~jdk_thread_server()
    {
        pool.get().please_stop();
	    pool.get().wait_for_stop(0);
    }

    bool run()
    {
        return factory.run() && pool.get().run();
    }

    bool update()
    {
        bool r=true;
        if(!factory.update() )
            r=false;
        if(!pool.get().update() )
            r=false;
        return r;
    }

	void poll()
		{
			factory.poll();
		}

    void stop()
    {
        pool.get().please_stop();
    }

    void kill()
    {
        pool.get().kill_all();
    }

    jdk_thread_server_factory  &factory;
	jdk_auto_ptr<jdk_thread_pool> pool;
};


template<class THREAD,class SHARED>
class jdk_thread_server_simple_factory : public jdk_thread_server_factory
{
public:
    explicit jdk_thread_server_simple_factory( 
		jdk_settings &settings_,
		jdk_nb_inet_server_socket &server_socket_
		)
	  : 
		settings( settings_ ),
		server_socket( server_socket_ )
	{
	}

	bool run()
	{
		return true;
	}

    bool update()
    {
        return true;
    }
		    
	jdk_thread *create_thread( jdk_thread_shared &shared, int id )
	{
	    return new THREAD( id, settings, *(SHARED *)&shared );
	}
	
	jdk_thread_pool_base *create_thread_pool()
	{
	    return new jdk_thread_pool( settings.get_long( "threads" ), *this );
	}

	jdk_thread_shared *create_thread_shared()
	{
	    return new SHARED( settings, server_socket );
	}		

    jdk_thread_server_base *create_thread_server()
    {
        return new jdk_thread_server( *this );
    }

    jdk_server_socket &get_server_socket()
    {
        return server_socket;
    }		
    
protected:
    jdk_settings &settings;    
	jdk_nb_inet_server_socket &server_socket;
private:
    jdk_thread_server_simple_factory( const jdk_thread_server_simple_factory<THREAD,SHARED> & ); // not allowed
    const jdk_thread_server_simple_factory<THREAD,SHARED> & operator = ( const jdk_thread_server_simple_factory<THREAD,SHARED> & ); // not allowed     
};


template <class FACTORYCLASS>
int jdk_thread_server_simplistic( 
								 int argc, 
								 char **argv, 
								 const char *settings_filename="settings.txt", 
								 jdk_setting_description *setting_descriptions=0 
								 )
{
    jdk_settings local_settings( setting_descriptions, argc, argv, settings_filename );

	jdk_log_setup(
				  local_settings.get_long( "log_type" ),
				  local_settings.get( "log_file" ),
				  local_settings.get_long( "log_detail" )
				  );				  


#if JDK_IS_UNIX_THREADS
	if( jdk_daemonize(
	                     (bool)local_settings.get_long("daemon"),
			             local_settings.get("name").c_str(),
				         local_settings.get("home").c_str(),
					     local_settings.get("pid_dir").c_str() ) <0
					     )
	{
		return 1;
	}

#endif

    jdk_settings settings( local_settings );
	jdk_nb_inet_server_socket server_socket( settings.get("interface"), settings.get_long("listen_count") );
    FACTORYCLASS *factory = new FACTORYCLASS( settings, server_socket );

    // drop root priviledges on unix if asked to
    jdk_thread_server_drop_root(local_settings);

	jdk_thread_server_base * server = factory->create_thread_server();
	
	// tell the server to run
	if( !server->run() )
	{
		jdk_log( JDK_LOG_ERROR, "Error starting server threads" );
		return 1;
	}
	
	jdk_log( JDK_LOG_INFO, "Started" );
	
	server->update();
	
#if JDK_IS_UNIX_THREADS
	while(1)
	{
		int sleep_time=3600;
		if( settings.find( "remote_update_rate" )!=-1 )
		{
			sleep_time = settings.get_long( "remote_update_rate" );
		}
		jdk_log( JDK_LOG_INFO, "Reload in %d seconds", sleep_time );
		for( int j=0; j<sleep_time; ++j )
		{
			sleep( 1 );
			
			server->poll();
			
			if( jdk_sigterm_caught)
			{
				jdk_log( JDK_LOG_INFO, "SIGTERM, About to exit" );					
#if 0					
				jdk_sigterm_caught=0;
				server->kill();
#endif
				delete server;
				delete factory;
				jdk_log( JDK_LOG_INFO, "SIGTERM, Exiting" );
				return 0;
			}
			if( jdk_sighup_caught )
			{
				jdk_sighup_caught=0;
				jdk_log( JDK_LOG_INFO, "SIGHUP" );
				break;
			}
		}
		jdk_log( JDK_LOG_INFO, "About to restart" );						
		server->update();
	}
#elif JDK_IS_WIN32
	
	while(1)
	{			
		int sleep_time=3600;
		if( settings.find( "remote_update_rate" )!=-1 )
		{
			sleep_time = settings.get_long( "remote_update_rate" );
		}
		
		Sleep( sleep_time * 1000 );
		jdk_log( JDK_LOG_INFO, "About to restart" );			
		server->update();			
	}		
#endif	
	
	
	jdk_log( JDK_LOG_NOTICE, "SHUTTING DOWN" );
	delete server;
	delete factory;
    return 0;    
}



#endif
