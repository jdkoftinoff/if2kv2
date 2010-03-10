#ifndef _JDK_INET_SERVER_H
#define _JDK_INET_SERVER_H

#include "jdk_socket.h"
#include "jdk_string.h"
#include "jdk_thread.h"

class jdk_inet_server_worker;
class jdk_inet_server_thread;
class jdk_inet_server_shared;


inline void jdk_inet_server_drop_root( int uid=-1, int gid=-1 )
{
#if JDK_IS_UNIX
	// set uid/gid is done only on initial execution, not after SIGHUP
	if( uid!=-1 && gid!=-1 )
	{
		if( setegid(gid)<0 || seteuid(uid)<0 )
		{
			jdk_log( JDK_LOG_ERROR, "Unable to set UID=%d GID=%d", uid, gid  );
			exit(1);
		}
    
    jdk_log( JDK_LOG_NOTICE, "UID=%d GID=%d", geteuid(), getegid() );		
	}
#endif	
}
class jdk_inet_server_worker
{
public:
  jdk_inet_server_worker() {}
  virtual ~jdk_inet_server_worker() {}
  
  virtual void run() = 0;
  virtual void update() = 0;
  virtual bool handle_connection( jdk_client_socket &sock ) = 0;
};


class jdk_inet_server_shared : public jdk_thread_shared
{
public:
  jdk_inet_server_shared();
  virtual ~jdk_inet_server_shared() {}
  
  jdk_mutex shared_mutex;
};


class jdk_inet_server_thread : public jdk_thread
{
public:
  explicit jdk_inet_server_thread( 
    jdk_inet_server_socket &server_socket_,
    jdk_inet_server_worker *worker_, 
    int id_ 
    )
    : 
    server_socket( server_socket_ ),
    worker( worker_ ), 
    id( id_ )
    {
    }
	
	~jdk_inet_server_thread()
    {
	    delete worker;
    }
	
	void update()
    {
	    jdk_synchronized( worker_mutex );
	    worker->update();
    }
	
protected:
  virtual void main()
    {
      //jdk_log( JDK_LOG_NOTICE, "id %d, in main()", id );
      {
        jdk_synchronized( worker_mutex );
        worker->run();
      }
      
	    while( !please_stop )
	    {
        //jdk_log( JDK_LOG_NOTICE, "id %d, waiting for accept()", id );
        if( server_socket.accept( &incoming ) )
		    {
          jdk_synchronized( worker_mutex );
          //jdk_log( JDK_LOG_NOTICE, "id %d, running worker handle_connection", id );
          if( !worker->handle_connection( incoming ) )
			    {
            break;
			    }
		    }
        incoming.close();
	    }
    }
  
private:
  jdk_inet_server_socket &server_socket;
  jdk_inet_server_worker *worker;
  jdk_mutex worker_mutex;
  int id;
  jdk_inet_client_socket incoming;
};


template <class WORKER, class SHARED >
class jdk_inet_server
{
public:
  
  explicit jdk_inet_server(
    SHARED &shared_,
    const jdk_string &local_binding,
    int num_threads=10,
    int uid=-1,
    int gid=-1
    )
    : 
    shared( shared_ ),
    server_socket( local_binding ),
    threads( num_threads )
    {

    }
  
	virtual ~jdk_inet_server()
    {
	    stop();
    }
	
	void run()
    {
      for( int i=0; i<threads.count(); ++i )
    	{
        jdk_inet_server_thread *t =
          new jdk_inet_server_thread(
            server_socket,
            new WORKER( shared, i ),
				    i
            );
        //jdk_log( JDK_LOG_NOTICE, "created thread at %08lx\n", (unsigned long)t );
		    threads.add( t );	
    	}
      
      for( int i=0; i<threads.count(); ++i )
	    {
        jdk_inet_server_thread *t = threads.get(i);
        
        //jdk_log( JDK_LOG_NOTICE, "about to run thread at %08lx\n", (unsigned long)t );			
		    if( !t || !t->run() )
		    {
          JDK_THROW_ERROR( "Error starting inet server thread", "" );
        }
	    }
    }
	
	void update()
    {
      {
        jdk_synchronized( shared.shared_mutex );      
        shared.update();
      }
	    for( int i=0; i<threads.count(); ++i )
	    {
        jdk_inet_server_thread *t = threads.get(i);
		    if( t )
		    {
          t->update();
		    }
	    }
    }
	
	void stop()
    {
	    for( int i=0; i<threads.count(); ++i )
	    {
        jdk_inet_server_thread *t = threads.get(i);
		    if( t )
		    {
          t->thread_pleasestop();
		    }
	    }
      jdk_sleep(1000);
	    for( int i=0; i<threads.count(); ++i )
	    {
        jdk_inet_server_thread *t = threads.get(i);
		    if( t )
		    {
          t->thread_cancel();
			    threads.remove( i );
		    }
	    }
    }
  
private:
  SHARED &shared;
  jdk_inet_server_socket server_socket;
  jdk_list<jdk_inet_server_thread> threads;    
};


#endif

