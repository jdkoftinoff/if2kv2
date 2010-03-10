#include "jdk_world.h"
#include "jdk_socketutil.h"
#include "jdk_log.h"


int jdk_socket_transfer_one_block(
  int my_id,
  jdk_client_socket *in,
  jdk_client_socket *out,
  int bytes_to_transfer,
  bool direction,
  bool do_dumps,
  FILE *dumpfile
  )
{
  char buf[4096];		
  
  if( bytes_to_transfer==-1 || bytes_to_transfer>(int)sizeof(buf) )
    bytes_to_transfer = (int)sizeof(buf);
  
  int len = in->read_data( buf, bytes_to_transfer );
  if( len<=0  )
  {
    //in->shutdown_read();
    //out->shutdown_write();
    jdk_log( JDK_LOG_DEBUG2, "(%d) socket closed during transfer (%s)", my_id, direction ? "client" : "server" );
    return 0;	// not a failure! we must forward on the closing, the other direction may still be going
  }
  jdk_log( JDK_LOG_DEBUG4, "(%d) %d bytes read from %s", my_id, len, direction ? "client" : "server" );
  if( do_dumps )
  {
    fwrite( buf, 1, len, dumpfile );
    fflush( dumpfile );
  }
  
  int bytes_sent = out->write_data_block( buf, len );
  if( bytes_sent==len )
  {
    jdk_log( JDK_LOG_DEBUG4, "(%d) %d bytes written to %s", my_id, len, direction ? "server" : "client" );
    return bytes_sent;
  }
  else
  {
    jdk_log( JDK_LOG_DEBUG4, "(%d) failed writing %d bytes to %s", my_id, len, direction ? "server" : "client" );
    return -1;
  }
}



void jdk_socket_transfer( 
  int my_id, 
  jdk_client_socket *in, 
  jdk_client_socket *out,
  bool do_dumps,
  FILE *incoming_dump,
  FILE *outgoing_dump
  )
{		
  
  while(
    (in->is_open_for_reading() && out->is_open_for_writing())
    || (out->is_open_for_reading() && in->is_open_for_writing())
    )
  {
    jdk_select_manager sel;
    
    if( in->is_open_for_reading() )
      sel.add_read( in );
    
    if( out->is_open_for_reading() )
      sel.add_read( out );
    
    sel.set_timeout(4);
    
    int cnt = sel.do_select();
    
    if( cnt<0 )
    {
      jdk_log( JDK_LOG_DEBUG1, "(%d) select<0, errno=%d", my_id, errno );
      break;
    }
    if( cnt>0 )
    {
      if( sel.can_read( in ) )
      {
        int len = jdk_socket_transfer_one_block( my_id, in, out, -1, true, do_dumps, incoming_dump );
        if( len<0 )
        {
          jdk_log( JDK_LOG_DEBUG2, "(%d) xfer in->out failed", my_id );
          break;
        }
        if( len==0 )
        {
          jdk_log( JDK_LOG_DEBUG2, "(%d) xfer in closed.", my_id );
          in->shutdown_read();
          out->shutdown_write();						
        }
        
      }
      if( sel.can_read( out ) )
      {
        int len = jdk_socket_transfer_one_block( my_id, out, in, -1, false, do_dumps, outgoing_dump );
        if( len<0 )
        {
          jdk_log( JDK_LOG_DEBUG2, "(%d) xfer out->in failed", my_id );
          break;
        }
        if( len==0 )
        {
          jdk_log( JDK_LOG_DEBUG2, "(%d) xfer out closed.", my_id );
          out->shutdown_read();
          in->shutdown_write();	
        }
        
      }																
    }							
  }		
}

int jdk_socket_transfer_one_block_with_processing(
  int my_id,
  jdk_client_socket *in,
  jdk_client_socket *out,
  int bytes_to_transfer,
  bool direction,
  jdk_socket_transfer_processor *proc,
  bool *in_closed_flag,
  bool do_dumps,
  FILE *dumpfile
  )
{
  char buf[8192];		
  
  if( bytes_to_transfer==-1 || bytes_to_transfer>(int)sizeof(buf) )
    bytes_to_transfer = (int)sizeof(buf);
  
  // try read data from 'in' until buf is full or socket is closed or 1 second elapses OR 'out' socket is readable
  int len=0;
  
  if( in_closed_flag )
    *in_closed_flag=false;
  
  while(bytes_to_transfer-len>0)
  {		
    jdk_select_manager sel;
    sel.add_read( in );
    if( out->is_open_for_reading() )
      sel.add_read( out );
    sel.set_timeout( 4 );
    int r = sel.do_select();
    if( r<0 )
    {
      // error doing select
      jdk_log( JDK_LOG_DEBUG2, "(%d) error doing select", my_id );
      return -1;
    }
    else if( r==0 )
    {
      // timeout!
      jdk_log( JDK_LOG_DEBUG4, "(%d) 4 second timeout during transfer", my_id );
      break;
    }
    else if( r>0 )
    {
      // something is readable.
      if( sel.can_read( in ) )
      {
        // try read more data into buf
        jdk_log( JDK_LOG_DEBUG4, "(%d) about to read 'in' dir=%d", my_id, (int)direction );
        
        int readlen = in->read_data(buf+len,bytes_to_transfer-len );
        if( readlen<0 )
        {
          // error
          jdk_log( JDK_LOG_DEBUG4, "(%d) error reading 'in' dir=%d", my_id, (int)direction );
          
          return -1;
        }
        else if( readlen==0 )
        {
          // no more to read cuz socket closed
          jdk_log( JDK_LOG_DEBUG4, "(%d) 'in' closed dir=%d", my_id, (int)direction );
          if( in_closed_flag )
          {
            *in_closed_flag = true;
          }
          break;
        }	
        else
        {
          // keep track of how much data was read
          jdk_log( JDK_LOG_DEBUG4, "(%d) received %d bytes from 'in' dir=%d", my_id, readlen , direction);
          
          len+=readlen;
        }
        
      }
      if( out->is_open_for_reading() )
      {				
        if( sel.can_read( out ) )
        {
          jdk_log( JDK_LOG_DEBUG4, "(%d) 'out' is readable, interrupting read of 'in' dir=%d", my_id, (int)direction );
          // other socket is readable now, so we must abort reading a full block
          break;
        }
      }
      
    }
  }
  
  
  if( len>0 && proc)
  {
    if( direction )
    {
      // true means client data is incoming
      len = proc->scan_incoming( buf, len, sizeof( buf ) );
    }
    else
    {
      // false means server data is incoming
      len = proc->scan_outgoing( buf, len, sizeof(buf) );
    }		
    
  }
  
  
  if( len<=0  )
  {
    //in->shutdown_read();
    //out->shutdown_write();
    jdk_log( JDK_LOG_DEBUG2, "(%d) socket closed during transfer (%s)", my_id, direction ? "client" : "server" );
    return 0;	// not a failure! we must forward on the closing, the other direction may still be going
  }
  
  
  jdk_log( JDK_LOG_DEBUG4, "(%d) %d bytes read from %s", my_id, len, direction ? "client" : "server" );
  if( do_dumps )
  {
    fwrite( buf, 1, len, dumpfile );
    fflush( dumpfile );
  }
  
  jdk_log( JDK_LOG_DEBUG4, "(%d) about to write %d bytes to out, dir=%d", my_id, len, (int)direction );
  int bytes_sent = out->write_data_block( buf, len );
  if( bytes_sent==len )
  {
    jdk_log( JDK_LOG_DEBUG4, "(%d) %d bytes written to %s", my_id, len, direction ? "server" : "client" );
    return bytes_sent;
  }
  else
  {
    jdk_log( JDK_LOG_DEBUG4, "(%d) failed writing %d bytes to %s", my_id, len, direction ? "server" : "client" );
    return -1;
  }
}


void jdk_socket_transfer_with_processing(
  int my_id, 
  jdk_client_socket *in, 
  jdk_client_socket *out,
  jdk_socket_transfer_processor *proc,
  bool do_dumps,
  FILE *incoming_dump,
  FILE *outgoing_dump
  )
{
  bool closed_flag=false;
  
  while(
    (in->is_open_for_reading() && out->is_open_for_writing())
    || (out->is_open_for_reading() && in->is_open_for_writing())
    )
  {
    jdk_select_manager sel;
    
    if( in->is_open_for_reading() )
      sel.add_read( in );
    
    if( out->is_open_for_reading() )
      sel.add_read( out );
    
    sel.set_timeout(4);
    
    int cnt = sel.do_select();
    
    if( cnt<0 )
    {
      jdk_log( JDK_LOG_DEBUG1, "(%d) select<0, errno=%d", my_id, errno );
      break;
    }
    if( cnt==0 )
    {
      jdk_log( JDK_LOG_DEBUG4, "(%d) 4 sec timeout (upper)", my_id );
    }
    if( cnt>0 )
    {
      if( sel.can_read( in ) )
      {
        jdk_log( JDK_LOG_DEBUG4, "(%d) in is readable", my_id );
        int xferlen=jdk_socket_transfer_one_block_with_processing( 
          my_id, 
          in, 
          out, 
          -1, 
          true,
          proc,
          &closed_flag,
          do_dumps, 
          incoming_dump
          );
        
        if( closed_flag )
        {
          jdk_log( JDK_LOG_DEBUG2, "(%d) xfer in->out said in closed", my_id );
          out->shutdown_write();
        }
        
        if( xferlen<0 )
        {
          jdk_log( JDK_LOG_DEBUG2, "(%d) xfer in->out failed", my_id );
          break;
        }
      }
      if( sel.can_read( out ) )
      {
        jdk_log( JDK_LOG_DEBUG4, "(%d) out is readable", my_id );
        
        int xferlen = jdk_socket_transfer_one_block_with_processing( 
          my_id, 
          out, 
          in, 
          -1,
          false,
          proc,
          &closed_flag,
          do_dumps, 
          outgoing_dump
          );
        
        if( closed_flag )
        {
          jdk_log( JDK_LOG_DEBUG2, "(%d) xfer out->in said out closed", my_id );
          in->shutdown_write();
        }
        if( xferlen<0 )
        {
          jdk_log( JDK_LOG_DEBUG2, "(%d) xfer out->in failed", my_id );
          break;
        }
        
      }																
    }							
  }		
}


