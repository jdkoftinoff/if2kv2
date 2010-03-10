#include "jdk_world.h"
#include "jdk_server.h"

#if JDK_HAS_THREADS

jdk_server::~jdk_server()
{
}







jdk_multi_server::jdk_multi_server( jdk_server_factory *factory_, int num_servers )
  : 
  factory( factory_ ),
  server_list( num_servers )
{
  for( int i=0; i<num_servers; ++i )
  {
    jdk_server *svr = factory->create_server();
    if( svr )
    {			
      add_server( svr );
    }		
  }
}

jdk_multi_server::~jdk_multi_server()
{
  // tell all servers to end gracefully	
  end_server();
}

// add a server to our list
int jdk_multi_server::add_server( jdk_server *svr )
{
  return server_list.add( svr );
}


// remove a server from our list
void jdk_multi_server::del_server( int id )
{
  server_list.remove( id );
}


// get a server by id number
jdk_server * jdk_multi_server::get_server( int id )
{
  return server_list.get( id );
}

const jdk_server *jdk_multi_server::get_server( int id ) const
{
  return server_list.get( id );
}


// get maximum number of servers
int jdk_multi_server::getmax_servers() const
{
  return server_list.count();
}

// getcurrent number of servers
int jdk_multi_server::count_servers() const
{
  return server_list.getnum();
}

// inherited from jdk_server:	

// initialize the servers
bool jdk_multi_server::init_server()
{
  for (int i=0; i<getmax_servers(); ++i )
  {
    jdk_server *s = get_server(i);
    if( s )
    {
      if( !s->init_server() )
      {
        // cant initialize the server so remove it
        del_server( i );
      }				
    }			
  }		
  
  return count_servers()>0;
}

// return true if any one of our servers returns true
bool jdk_multi_server::can_start_server() const
{
  for (int i=0; i<getmax_servers(); ++i )
  {
    const jdk_server *s = get_server(i);
    if( s )
    {
      if( s->can_start_server() )
      {
        return true;
      }
    }
  }
  
  return false;	
}


// start one of the servers processing the specified socket
bool jdk_multi_server::start_server( jdk_client_socket *client )
{
  for (int i=0; i<getmax_servers(); ++i )
  {
    jdk_server *s = get_server(i);
    if( s )
    {
      if( s->can_start_server() )
      {				
        if( s->start_server(client) )
        {
          // found a server and started it!
          return true;
        }				
      }			
    }
    
  }		
  
  // we couldnt find a server to manage this client. 
  return false;
}


// end the servers
void jdk_multi_server::end_server()
{
  for (int i=0; i<getmax_servers(); ++i )
  {
    jdk_server *s = get_server(i);
    if( s )
    {
      s->end_server();
      del_server(i);
    }
  }				
}


// tell select manager what timeout and fds we are interested in.	
void jdk_multi_server::prepare( jdk_select_manager *mgr )
{
  for (int i=0; i<getmax_servers(); ++i )
  {
    jdk_server *s = get_server(i);
    if( s )
    {
      // ask every valid server to contribute
      s->prepare( mgr );
    }
  }						
}


// dispatch events based on results from select
// if dispatch() returns false the server will be shut down	
bool jdk_multi_server::dispatch( jdk_select_manager *mgr )
{
  for (int i=0; i<getmax_servers(); ++i )
  {
    jdk_server *s = get_server(i);
    if( s )
    {
      // if our server returned false then shut it down 
      // and delete it.
      s->dispatch( mgr );
      // dont delete.
    }
  }
  
  // return false if we have no servers left
  return count_servers() > 0;
}


// timeout() is called if the select timeout occurs.
// if timeout() returns false the server will be shut down.
bool jdk_multi_server::timeout()
{
  for (int i=0; i<getmax_servers(); ++i )
  {
    jdk_server *s = get_server(i);
    if( s )
    {
      // if our server returned false then shut it down 
      // and delete it.
      if( !s->timeout() )
      {
        // dont do this, just call stop and restart it
        
        //s->end_server();
        //del_server(i);
        
        s->stop();
        s->start();
      }
    }
  }
  
  // return false if we have no servers left
  return count_servers() > 0;		
}




jdk_server_manager::jdk_server_manager( jdk_server_factory *factory_ )
  : 
  factory( factory_ ),
  main_server( factory_->create_main_server() )
{
}


jdk_server_manager::~jdk_server_manager()
{
  if( main_server )
  {
    main_server->end_server();
    delete main_server;		
  }
}

void jdk_server_manager::tick()
{
}


bool jdk_server_manager::run_server( jdk_server_socket *svr_sock,
                                     volatile bool *please_stop )
{
  if( !main_server )
  {
    return false;
  }
  
  if( !main_server->init_server() )
  {
    return false;
  }
  
  jdk_select_manager sel;
  do
  {
    sel.clear();
    // default 1800 second time out. main_server can override this.
    sel.set_timeout( 1800 );
    
    main_server->prepare( &sel );
    
    // if we have a server socket, then also check if we can accept a connection on it.
    if( svr_sock )
    {
      // only wait for incoming server connections if we actually have a server
      // to give it to.
      
      if( main_server->can_start_server() )
      {
        // yes, we have a socket and a free server.
        sel.add_read( svr_sock );
      }
    }
    
    
    int r=sel.do_select();
    
    if( r==0 )
    {
      // timeout!
      
      tick();
      
      if( !main_server->timeout() )
      {
        // the server wants to end!
        break;
      }			
    }
    
    if( r>0 )
    {
      // if we have a server socket, then also check if we can accept a connection on it.
      
      if( svr_sock )
      {
        if( sel.can_read( svr_sock ) )
        {
          // yes! we have an incoming connection!
          
          // accept the connection.
          
          jdk_client_socket *client=svr_sock->accept_create();
          
          // did we get the connection?
          if( client )
          {						
            // start a server on this connection.
            
            if( main_server->start_server( client ) )
            {
              // the server was started.
              // we continue here so that the select loop can run again
              // because our select response is out of date now and
              // the started server has not got its prepare() in.
              continue;
            }						
            else
            {
              // oh oh someone lied to us.  the main_server said
              // that he could start up but he wasnt able to.
              // kill the connection
              
              delete client;
            }
          }											
        }
      }
      
      
      // we have some event to tell the server about
      if( !main_server->dispatch( &sel ) )
      {
        // the server wants to end!
        break;
      }
      
    }
    
  } while( !please_stop || (!*please_stop) );
  
  // tell the server to end
  
  main_server->end_server();
  
  // we are done!
  return true;
}





jdk_server_manager_thread::jdk_server_manager_thread( 
  jdk_server_factory *factory_,
  jdk_server_socket *server_socket_
  )
  : 
  factory( factory_ ),
  manager( factory_->create_server_manager() ),
  server_socket( server_socket_ )
{
}


jdk_server_manager_thread::~jdk_server_manager_thread()
{
  delete manager;
}


void jdk_server_manager_thread::main()
{
  // tell the server manager to run.
  if( manager )
    manager->run_server(server_socket,&please_stop);
}





jdk_server_factory::jdk_server_factory()
{
}

jdk_server_factory::~jdk_server_factory()
{
}


#endif
