#include "jdk_world.h"
#include "jdk_nameserver.h"

#if JDK_IS_UNIX && 0

jdk_nameserver_request_packet::jdk_nameserver_request_packet()
  :
  magic( JDK_NAMESERVER_MAGIC ),
  id1(0),
  id2(0)
{
  *name=0;
}




jdk_nameserver_response_packet::jdk_nameserver_response_packet()
  :
  magic( JDK_NAMESERVER_MAGIC ),
  id1(0),
  id2(0)
{
  *name=0;
  *ip=0;
}






jdk_nameserver_event::jdk_nameserver_event()
{
}

jdk_nameserver_event::~jdk_nameserver_event()
{
}




jdk_nameserver_worker::jdk_nameserver_worker(
  jdk_client_socket *request_rd_,
  jdk_client_socket *response_wr_
  )
  :
  request_rd( request_rd_ ),
  response_wr( response_wr_ )
{
#if JDK_IS_UNIX
  
  // unix systems need to fork() to allow multiple async lookups
  thread_usefork();
#endif	
}

jdk_nameserver_worker::~jdk_nameserver_worker()
{
}

void jdk_nameserver_worker::main()
{	
  while(1)
  {
    jdk_nameserver_request_packet request;
    jdk_nameserver_response_packet response;
    
    // wait for a request to come in.
    
    int readsz = request_rd->read_data( &request, sizeof(request) );
    
    if( readsz==0 )
    {
      request_rd->close();
      // request socket was closed! end thread!
      return;
    }
    
    if( readsz==sizeof(request) && request.magic == JDK_NAMESERVER_MAGIC )
    {
      // we got a valid request!
      
      response.id1 = request.id1;
      response.id2 = request.id2;
      
      strncpy( response.name, request.name, sizeof(response.name)-1 );
      response.name[ sizeof( response.name )-1 ] = '\0';
      
      // try look it up
      
      struct sockaddr_in addr;
      
//			fprintf( stderr, "request is '%s'\n", request.name );
      
      if( jdk_socket_lpgethostbyname( request.name, &addr )==0 )
      {
        // successful lookup! convert ip addr to string
        
        unsigned long ip = ntohl( addr.sin_addr.s_addr );
        
        sprintf( response.ip, "%ld.%ld.%ld.%ld", 
                 (ip>>24)&0xff,
                 (ip>>16)&0xff,
                 (ip>>8)&0xff,
                 (ip>>0)&0xff
          );				
        
        
      }
      else
      {
        // failure on lookup! return empty string
        response.ip[0] ='\0';
      }
      
//			fprintf( stderr, "response is '%s'\n", response.ip );			
      
      // send response back to whoever requested it
      
      response_wr->write_data_block( &response, sizeof( response ) );			
    }		
  };
}






jdk_nameserver::jdk_nameserver( int pool_size_ )
  :
  pool_size( pool_size_ )
{
  jdk_make_pipe_sockets( 
    &request_pipe_rd,
    &request_pipe_wr
    );
  jdk_make_pipe_sockets( 
    &response_pipe_rd,
    &response_pipe_wr
    );							  
}

jdk_nameserver::~jdk_nameserver()
{
}

int jdk_nameserver::run()
{
  int cnt=0;
  
  for( int i=0; i<pool_size; ++i )
  {
    jdk_nameserver_worker *nsw;
    
    nsw = new jdk_nameserver_worker( 
      &request_pipe_rd,
      &response_pipe_wr
      );
    
    if( nsw )
    {
      nsw->thread_plansuicide(true);
      nsw->run();
      ++cnt;
    }
    
  }	
  
  return cnt;
}

bool jdk_nameserver::request(
  jdk_nameserver_event *cb,
  const char *hostname
  )
{
  // create a request packet
  
  jdk_nameserver_request_packet req;
  
  // set id1 to be the lower 32 bits of cb
  
  req.id1 = (  ((jdk_int64)cb) &0xffffffff);
  
  // set id2 to be the upper 32 bits of cb
  
  req.id2 = (( ((jdk_int64)cb) >>32) & 0xffffffff);
  
  // set name to the host name we want
  strncpy( req.name, hostname, sizeof(req.name)-1 );
  req.name[sizeof(req.name)-1] = '\0';
  
  // send it off to one of our pooled nameservers via a pipe
  
  return request_pipe_wr.write_data_block( &req, sizeof(req) )==sizeof(req);
}


void jdk_nameserver::prepare( jdk_select_manager *mgr )
{
  // add our response_pipe_rd to the manager for reading
  
  mgr->add_read( &response_pipe_rd );
  
  // set our timeout to 15 minutes - we ping our nameservers every 15 minutes
  
  mgr->set_timeout( 15 * 60 );		
}


bool jdk_nameserver::dispatch( jdk_select_manager *mgr )
{
  // are we able to read a response?
  
  if( mgr->can_read( &response_pipe_rd ) )
  {
    // yes! try read a response packet
    
    jdk_nameserver_response_packet response;
    
    int readsz = response_pipe_rd.read_data( &response, sizeof(response) );
    
    
    if( readsz==sizeof( response ) && response.magic==JDK_NAMESERVER_MAGIC )
    {
      // got a valid response!
      // figure out the ptr to the event handler
      
      jdk_nameserver_event *ev = (jdk_nameserver_event *)((jdk_int64)response.id1 + ( ((jdk_int64)response.id2)<<32));
      
      // if the pointer is null then it is just a ping response. forget it!
      
      // if the pointer is valid then call the event handler!
      
      if( ev )
      {
        ev->nameserver_response( response.name, response.ip );	
      }
      
    }
    else
    {
      // oh oh bad read value - pipe is closed or corrupted. return false
      // which means end this task
      response_pipe_rd.close();
      return false;
    }
    
    
  }
  
  return true;
}


bool jdk_nameserver::timeout()
{
  // we got a time out, this means we ping our nameserver to
  // keep our connection alive
  
  // create a request packet
  
  jdk_nameserver_request_packet req;
  
  // set id1 to be 0
  
  req.id1 = 0;
  
  // set id2 to be 0
  
  req.id2 = 0;
  
  // set name to be null host name
  req.name[0] = '\0';
  
  // send it off to one of our pooled nameservers via a pipe 
  // if the write failed then return false, asking for exit of the socket_task
  return request_pipe_wr.write_data_block( &req, sizeof(req) )==sizeof(req);
  
}



#endif
