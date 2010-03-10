#ifndef _JDK_SERVER_H
#define _JDK_SERVER_H

#include "jdk_thread.h"
#include "jdk_socket.h"
#include "jdk_list.h"
#if JDK_HAS_THREADS
class jdk_server;
	class jdk_multi_server; 	

class jdk_server_manager;

class jdk_server_manager_thread;	// public jdk_thread

class jdk_server_factory;

//----------------------------------------------------------------------


// a jdk_server is an abstract base class for a server class that 
// can manage a number of socket connections

class jdk_server : public jdk_socket_task 
{
 public:
	jdk_server()
	{
	}
	
	virtual ~jdk_server();

	// initialize the servers
	// returns false if the server cant be initialized for 
	// some reason.
   	virtual bool init_server() =0;

	// returns true if this server is 'startable' ie not busy with another client
	virtual bool can_start_server() const =0;
	
	// start the server processing the specified socket	  
   	virtual bool start_server( jdk_client_socket *s ) = 0;
	
	// end the server
	virtual void end_server() = 0;
	
	// tell select manager what timeout and fds we are interested in.	
	virtual void prepare( jdk_select_manager *mgr ) =0;
	
	// dispatch events based on results from select
	// if dispatch() returns false the server will be shut down	
	virtual	bool dispatch( jdk_select_manager *mgr ) =0;
	
	// timeout() is called if the select timeout occurs.
	// if timeout() returns false the server will be shut down.
	virtual bool timeout() = 0;

 private:

	jdk_server( const jdk_server &other ); // not allowed!
	const jdk_server & operator = ( const jdk_server & );	
}; 

// a jdk_multi_server object manages the start, end, and preparation of multiple
// jdk_server objects within the same thread. it itself is a jdk_server so it can
// be used in place of one.

class jdk_multi_server : public jdk_server 
{
  explicit jdk_multi_server( const jdk_multi_server &);
  const jdk_multi_server & operator = ( const jdk_multi_server & );
 public:
	explicit jdk_multi_server( jdk_server_factory *factory_, int num_servers );
		
   	virtual ~jdk_multi_server();
	

// inherited from jdk_server:	

	// initialize the servers
	void start(){}
	void stop() { end_server(); }

   	bool init_server();

	// returns true if any server is 'startable' ie not busy with another client
	bool can_start_server() const;
	
	// start one server processing the specified socket 
   	bool start_server( jdk_client_socket *s );
		
	// end the servers
   	void end_server();
	
	// tell select manager what timeout and fds we are interested in.	
	void prepare( jdk_select_manager *mgr );
	
	// dispatch events based on results from select
	// if dispatch() returns false the server will be shut down	
   	bool dispatch( jdk_select_manager *mgr );
	
	// timeout() is called if the select timeout occurs.
	// if timeout() returns false the server will be shut down.
	virtual bool timeout();

 protected:	
	// add a server to our list
   	int add_server( jdk_server *svr );
	
	// remove a server from our list
   	void del_server( int id );
	
	// get a server by id number
	jdk_server * get_server( int id );
	
	const jdk_server *get_server( int id ) const;
	
	// get maximum number of servers
	int getmax_servers() const;
	
	// getcurrent number of servers
	int count_servers() const;
	
 private:
	
	jdk_server_factory *factory;
	jdk_list< jdk_server > server_list;
}; 

// a jdk_server_manager object. manages the main loop of a single thread, running
// a single jdk_server object until it is done.

class jdk_server_manager 
{
 public:
	jdk_server_manager( jdk_server_factory *factory_ );
	virtual ~jdk_server_manager();
	
	virtual bool run_server( jdk_server_socket *s=0, volatile bool *please_stop=0 );
	  
 protected:
	
	virtual void tick();
	
	jdk_server_factory *factory;
	jdk_server *main_server;
	
	
private:
	jdk_server_manager( const jdk_server_manager &o ); // not allowed!
	const jdk_server_manager & operator = (const jdk_server_manager & );
}; 

// a jdk_server_manager_thread is a jdk_thread that runs a jdk_server_manager
// within its own thread, until the jdk_server_manager finishes.

class jdk_server_manager_thread : public jdk_thread 
{
 public:
	jdk_server_manager_thread( jdk_server_factory *factory_, jdk_server_socket *server_socke_ );
	virtual ~jdk_server_manager_thread();
	
 protected:
	void main();
	
	jdk_server_factory *factory;
	jdk_server_manager *manager;
	
	jdk_server_socket *server_socket;
	
 private:
	jdk_server_manager_thread( const jdk_server_manager_thread &o ); // not allowed
	const jdk_server_manager_thread &operator = ( const jdk_server_manager_thread & );
}; 

class jdk_server_factory
{
public:
	jdk_server_factory();
	virtual ~jdk_server_factory();	
	
	virtual jdk_client_socket *create_client_socket() = 0;
	virtual jdk_server_socket *create_server_socket() = 0;
	virtual jdk_server *create_main_server() = 0;
	virtual jdk_server *create_server() = 0;	
	virtual jdk_server_manager *create_server_manager() = 0;
	virtual jdk_server_manager_thread *create_server_manager_thread() = 0;

}; 


#endif
#endif



