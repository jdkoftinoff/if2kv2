#include "jdk_world.h"
#include "if2k_mini_config.h"
#include "if2k_mini_tunnel.h"


bool if2k_mini_tunnel::continue_running() const
{
  bool conduit_to_client_is_open =
      incoming.is_open_for_writing() && outgoing.is_open_for_reading();

  bool conduit_to_server_is_open =
      outgoing.is_open_for_writing() && incoming.is_open_for_reading();

  bool conduit_is_open =
      conduit_to_server_is_open || conduit_to_client_is_open;

  bool data_to_be_sent_to_client =
      incoming.is_open_for_writing() && data_to_client.get_data_length()>0;

  bool data_to_be_sent_to_server =
      outgoing.is_open_for_writing() && data_to_server.get_data_length()>0;

  bool data_to_be_sent =
      data_to_be_sent_to_client || data_to_be_sent_to_server;

  jdk_log( JDK_LOG_DEBUG4, "tunnel: ->client: %d  ->server: %d  clientdata: %d  serverdata %d",
           conduit_to_client_is_open,
           conduit_to_server_is_open,
           data_to_be_sent_to_client,
           data_to_be_sent_to_server
      );
  

  return conduit_is_open || data_to_be_sent;
}

bool if2k_mini_tunnel::handle_transaction(
  jdk_uint64 max_received_client_byte_count_,
  jdk_uint64 max_received_server_byte_count_,
  jdk_buf *client_prefix_data,
  jdk_buf *server_prefix_data
  )
{
 
  jdk_log( JDK_LOG_DEBUG2, "Starting if2k_mini_tunnel transaction" );
  bool r=false; 
  max_received_client_byte_count = max_received_client_byte_count_;
  max_received_server_byte_count = max_received_server_byte_count_;
 
  jdk_staticbuf<4096> buf;

  jdk_select_manager sel;

  incoming_shutdown=false;
  outgoing_shutdown=false;

  if( client_prefix_data && client_prefix_data->get_data_length() >0 )
  {
    jdk_log( JDK_LOG_DEBUG2, "prefix data from client: %d bytes", client_prefix_data->get_data_length() );
    if( handle_client_incoming_data( *client_prefix_data ) )
    {
      outgoing_shutdown=true;
    }
  }

  if( server_prefix_data && server_prefix_data->get_data_length() >0 )
  {
    jdk_log( JDK_LOG_DEBUG2, "prefix data from server: %d bytes", server_prefix_data->get_data_length() );
    if( handle_server_incoming_data( *server_prefix_data ) )
    {
      incoming_shutdown=true;
    }
  }

  while( continue_running() )
  {
    sel.clear();
    
    if( incoming.is_open_for_reading() && data_to_server.get_data_length()<8192 )
    {
      jdk_log( JDK_LOG_DEBUG4, "expecting incoming read" );
      sel.add_read( incoming );
    }

    if( outgoing.is_open_for_reading() && data_to_client.get_data_length()<8192  )
    {
      jdk_log( JDK_LOG_DEBUG4, "expecting outgoing read" );
      sel.add_read( outgoing );
    }

    if( incoming_shutdown )
      received_client_holdoff=0;

    bool has_data_to_client = (incoming.is_open_for_writing() && data_to_client.get_data_length()>0);
    bool has_data_to_server = (outgoing.is_open_for_writing() && data_to_server.get_data_length()>0);

//    if( data_to_server.get_data_length() > received_client_holdoff && outgoing.is_open_for_writing() && !outgoing_shutdown)
    if( has_data_to_server )
    {
      jdk_log( JDK_LOG_DEBUG4, "expecting write for server data: %d", data_to_server.get_data_length() );
      sel.add_write( outgoing );
    }

    if( outgoing_shutdown )
      received_server_holdoff=0;

//    if( data_to_client.get_data_length() > received_server_holdoff && incoming.is_open_for_writing() && !incoming_shutdown)
    if( has_data_to_client )
    {
      jdk_log( JDK_LOG_DEBUG4, "expecting write for client data: %d", data_to_client.get_data_length() );
      sel.add_write( incoming );
    }

    
    if( !has_data_to_client && !has_data_to_server && (incoming_shutdown || outgoing_shutdown) )
    {
      jdk_log( JDK_LOG_DEBUG4, "Finished: %d %d %d %d", has_data_to_client, has_data_to_server, incoming_shutdown, outgoing_shutdown );
      break;
    }
    
    jdk_log( JDK_LOG_DEBUG4, "Has data to client: %d (%d) Has data to server: %d (%d)", 
             has_data_to_client, data_to_client.get_data_length(),
             has_data_to_server, data_to_server.get_data_length()
        );

    sel.set_timeout( 2 );   

    int r=sel.do_select();
    if( r<0 )
    {
      jdk_log( JDK_LOG_DEBUG4, "Finished: (select -1)");
      break;
    }

    
    if( r==0 )
    {
      // timeout
        jdk_log( JDK_LOG_DEBUG4, "tick" );
    }

    if( r>0 )
    {
      if( sel.can_read( incoming ) ) 
      {
        jdk_log( JDK_LOG_DEBUG4, "incoming readable" );
        if( outgoing.is_open_for_writing() )
        {
          size_t max_to_read = data_to_server.get_available_space();
          if( max_to_read>buf.get_buf_length() )
            max_to_read=buf.get_buf_length();
          
          ssize_t len = incoming.recv_data( buf.get_data(), max_to_read );
          
          if( len>0 )
          {
            buf.set_data_length(len);
            jdk_log( JDK_LOG_DEBUG4, "Received %d bytes from client", len );
            
            if( handle_client_incoming_data(buf) )
            {
              r=true; // transaction is finished
              jdk_log( JDK_LOG_DEBUG4, "Finished: client_incoming request");
              break;
            }
          }
          else
          {
            outgoing_shutdown=true;
            jdk_log( JDK_LOG_DEBUG3, "Closing incoming and scheduling outgoing_shutdown (client read<=0)" );            
            incoming.close();
          }
        }
        else
        {
          jdk_log( JDK_LOG_DEBUG3, "closing client (server write is not open)" );
          incoming.close();
        }
      }
        
      if( sel.can_read( outgoing ) ) //&& data_to_client.get_available_space()>0 && !incoming_shutdown )
      {
        jdk_log( JDK_LOG_DEBUG4, "outgoing readable" );
        if( incoming.is_open_for_writing() )
        {
          size_t max_to_read = data_to_client.get_available_space();
          if( max_to_read>buf.get_buf_length() )
            max_to_read=buf.get_buf_length();
          
          ssize_t len = outgoing.recv_data( buf.get_data(), max_to_read );
          if( len>0 )
          {
            jdk_log( JDK_LOG_DEBUG4, "Received %d bytes from server", len );
            buf.set_data_length(len);
            
            if( handle_server_incoming_data(buf) )
            {
              r=true; // transaction is finished
              jdk_log( JDK_LOG_DEBUG4, "Finished: server_incoming request");
              break;
            }
          }
          else
          {
            incoming_shutdown=true;
            jdk_log( JDK_LOG_DEBUG3, "Closing outgoing and scheduling incoming_shutdown (server read<=0)" );
            outgoing.close();
          }
        }
        else
        {
          jdk_log( JDK_LOG_DEBUG3, "closing outgoing (client not writable)");
          outgoing.close();
        }
      }

      if( sel.can_write( incoming ) )
      {
        jdk_log( JDK_LOG_DEBUG4, "incoming writable" );
        if( data_to_client.get_data_length()==0 )
        {
          jdk_log( JDK_LOG_ERROR, "ERROR client is writable but no data to send" );
          JDK_THROW_ERROR( "ERROR client is writable but no data to send","" );
        }

        jdk_dynbuf tmp;
        data_to_client.peek( tmp );

        ssize_t len = incoming.send_buf( tmp );
        if( len>0 )
        {
          jdk_log( JDK_LOG_DEBUG4, "Sent %d bytes to client", len );          
          data_to_client.skip( (int)len );
          actual_sent_to_client_byte_count+=len;
          received_server_holdoff -= (int)len;
          if( received_server_holdoff<0 )
            received_server_holdoff=0;
        }
        else
        {
          jdk_log( JDK_LOG_DEBUG3, "shutdown_write() to client (client write<=0)", len );
          incoming.shutdown_write();
        }
      }
      
      if( sel.can_write( outgoing ) )
      {
        jdk_log( JDK_LOG_DEBUG4, "outgoing writable" );
        if( data_to_server.get_data_length()==0 )
        {
          jdk_log( JDK_LOG_ERROR, "ERROR server is writable but no data to send" );
          JDK_THROW_ERROR( "ERROR server is writable but no data to send", "" );
        }

        jdk_dynbuf tmp;
        data_to_server.peek( tmp );

        ssize_t len = outgoing.send_buf( tmp );
        if ( len>0 )
        {
          jdk_log( JDK_LOG_DEBUG4, "Sent %d bytes to server", len );
          data_to_server.skip( len );

          actual_sent_to_server_byte_count +=len;
          received_client_holdoff -= len;
          if( received_client_holdoff<0 )
            received_client_holdoff=0;

        }
        else
        {
          jdk_log( JDK_LOG_DEBUG3, "shutdown_write() to server (server write<=0)", len );
          outgoing.shutdown_write();
        }
      }

      if( incoming_shutdown && data_to_client.get_data_length()==0 )
      {
        if( incoming.is_open_for_reading() )
        {
          jdk_log( JDK_LOG_DEBUG4, "shutdown_read() to client" );
          incoming.shutdown_read();
        }
        if( outgoing.is_open_for_writing() )
        {
          jdk_log( JDK_LOG_DEBUG4, "shutdown_write() to server" );
          outgoing.shutdown_write();
        }
      }
      
      if( outgoing_shutdown && data_to_server.get_data_length()==0 )
      {
        if( outgoing.is_open_for_reading() )
        {
          jdk_log( JDK_LOG_DEBUG4, "shutdown_read() to server" );
          outgoing.shutdown_read();
        }
        if( incoming.is_open_for_writing() )
        {
          jdk_log( JDK_LOG_DEBUG4, "shutdown_write() to client" );
          incoming.shutdown_write();
        }
      }      
    }
  }
  jdk_log( JDK_LOG_DEBUG2, 
           "Ending if2k_mini_tunnel transaction (%d) %lld in, %lld out", 
           (int)r, actual_sent_to_client_byte_count, 
           actual_sent_to_server_byte_count
      );
  return r;
}

bool if2k_mini_tunnel::send_data_to_client(  const jdk_buf &data )
{
  bool r=false; 
  if( max_received_server_byte_count==0 )
  {
    // no limit

    for( size_t i=0; i<data.get_data_length(); ++i )
    {
      data_to_client.put( (char)data.get_data()[i] );
    }
  }
  else
  {
    size_t a = data_to_client.get_available_space();
    size_t b = data.get_data_length();
    jdk_uint64 c = max_received_server_byte_count;
    
    jdk_uint64 minimum = (a<b ? a: b);
    minimum = (minimum<c ? minimum : c );

    
    for( jdk_uint64 i=0; i<minimum; ++i )
    {
      data_to_server.put( (char)data.get_data()[i] );
    }
    
    max_received_server_byte_count -= minimum;
    if( max_received_server_byte_count==0 )
    {
      r=true; // finished transaction mode
    }
  }
  return r;
}

bool if2k_mini_tunnel::send_data_to_server( const jdk_buf &data )
{
  bool r=false;
  if( max_received_client_byte_count==0 )
  {
    // no limit
    for( size_t i=0; i<data.get_data_length(); ++i )
    {
      data_to_server.put( (char)data.get_data()[i] );
    }
  }
  else
  {
    size_t a = data_to_server.get_available_space();
    size_t b = data.get_data_length();
    jdk_int64 c = max_received_client_byte_count;
    
    jdk_int64 minimum = (a<b ? a: b);
    minimum = (minimum<c ? minimum : c );
    
    for( jdk_int64 i=0; i<minimum; ++i )
    {
      data_to_server.put( (char)data.get_data()[i] );
    }
    
    max_received_client_byte_count -= minimum;
    if( max_received_client_byte_count==0 )
    {
      r=true; // finished transaction
    }
    
  }
  return r;
}

bool if2k_mini_tunnel::handle_client_incoming_data(  jdk_buf &data )
{
  bool r=send_data_to_server( data );
  return r;
}

bool if2k_mini_tunnel::handle_server_incoming_data(  jdk_buf &data )
{
  bool r=send_data_to_client( data );
  return r;
}


void if2k_mini_line_tunnel::clear()
{
  if2k_mini_tunnel::clear();
  client_linesplitter.clear();
  server_linesplitter.clear();
}

bool if2k_mini_line_tunnel::handle_client_incoming_data(  jdk_buf &data )
{
  bool r=false;

  client_linesplitter.set_buf( data );

  while( client_linesplitter.scan_for_line(0,line_ign,line_term) )
  {
    r=handle_client_incoming_line( client_linesplitter.get_line_buf() );
  }
  return r;
}

bool if2k_mini_line_tunnel::handle_server_incoming_data(  jdk_buf &data )
{
  bool r=false;

  server_linesplitter.set_buf( data );

  while( server_linesplitter.scan_for_line(0,line_ign,line_term) )
  {
    r=handle_server_incoming_line( server_linesplitter.get_line_buf() );
  }
  return r; 
}

bool if2k_mini_line_tunnel::handle_client_incoming_line(  char *line )
{
//  jdk_log( JDK_LOG_DEBUG4, "sending to server: \"%s\"", line );
  return send_string_to_server( line ) || send_string_to_server(line_term_to_send);
}

bool if2k_mini_line_tunnel::handle_server_incoming_line(  char *line )
{
//  jdk_log( JDK_LOG_DEBUG4, "sending to client: \"%s\"", line );
  return send_string_to_client( line ) || send_string_to_client(line_term_to_send);
}

bool if2k_mini_line_tunnel::send_string_to_client( const char *line )
{
  size_t len = strlen(line);
  jdk_buf b( (char*)line, len, len );
  return send_data_to_client( b );
}

bool if2k_mini_line_tunnel::send_string_to_server( const char *line )
{
  size_t len = strlen(line);
  jdk_buf b( (char*)line, len, len );
  return send_data_to_server( b ) ;
}


