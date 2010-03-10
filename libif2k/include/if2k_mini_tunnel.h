#ifndef __IF2K_MINI_TUNNEL_H
#define __IF2K_MINI_TUNNEL_H

#include "jdk_world.h"
#include "if2k_mini_config.h"
#include "jdk_socket.h"
#include "jdk_elastic_queue.h"
#include "jdk_linesplit.h"

class if2k_mini_tunnel
{
  if2k_mini_tunnel( const if2k_mini_tunnel &o );
  const if2k_mini_tunnel & operator = ( const if2k_mini_tunnel &o );

public:
  explicit if2k_mini_tunnel(
      jdk_client_socket &incoming_,
      jdk_client_socket &outgoing_,
      int queue_sz=8192,
      int received_client_holdoff_=0, 
      int received_server_holdoff_=0
      )
    :
      incoming( incoming_ ),
      outgoing( outgoing_ ),
      data_to_client(queue_sz),
      data_to_server(queue_sz),
      received_client_holdoff( received_client_holdoff_ ),
      received_server_holdoff( received_server_holdoff_ ),
      max_received_client_byte_count(0),
      max_received_server_byte_count(0),
      actual_sent_to_client_byte_count(0),
      actual_sent_to_server_byte_count(0)
    {
    }

  virtual ~if2k_mini_tunnel() {}

  virtual void clear()
    {
      data_to_server.clear();
      data_to_client.clear();
      max_received_server_byte_count = 0;
      max_received_client_byte_count = 0;
      actual_sent_to_server_byte_count = 0;
      actual_sent_to_client_byte_count = 0;
    }

  virtual bool continue_running() const;

  virtual bool handle_transaction( 
    jdk_uint64 max_received_client_byte_count,
    jdk_uint64 max_received_server_byte_count,
    jdk_buf *client_prefix_data,
    jdk_buf *server_prefix_data
    );
  
  virtual bool handle_client_incoming_data( jdk_buf &data );
  virtual bool handle_server_incoming_data( jdk_buf &data );
  virtual bool send_data_to_client( const jdk_buf &data );
  virtual bool send_data_to_server( const jdk_buf &data );

protected:

  jdk_client_socket &incoming;
  jdk_client_socket &outgoing;

  jdk_elastic_queue<unsigned char> data_to_client;
  jdk_elastic_queue<unsigned char> data_to_server;

  int received_client_holdoff;
  int received_server_holdoff;

  jdk_uint64 max_received_client_byte_count;
  jdk_uint64 max_received_server_byte_count;

  jdk_uint64 actual_sent_to_client_byte_count;
  jdk_uint64 actual_sent_to_server_byte_count;

  bool incoming_shutdown;
  bool outgoing_shutdown;
};


class if2k_mini_line_tunnel : public if2k_mini_tunnel
{
  if2k_mini_line_tunnel( const if2k_mini_line_tunnel &o );
  const if2k_mini_line_tunnel & operator = ( const if2k_mini_line_tunnel &o );

public:
  explicit if2k_mini_line_tunnel( 
      jdk_client_socket &incoming_,
      jdk_client_socket &outgoing_,
      int linesz=4096, 
      const char *line_ign_ = "\r", 
      const char *line_term_="\n", 
      const char *line_term_to_send_="\r\n" 
      ) 
      :
      if2k_mini_tunnel( incoming_, outgoing_ ),
      server_linesplitter(linesz),
      client_linesplitter(linesz),
      line_ign( line_ign_ ),
      line_term( line_term_ ),
      line_term_to_send( line_term_to_send_ )
    {
    }

  virtual void clear();
  virtual bool handle_client_incoming_data( jdk_buf &data );
  virtual bool handle_server_incoming_data( jdk_buf &data );

  virtual bool handle_client_incoming_line( char *line );
  virtual bool handle_server_incoming_line( char *line );

  virtual bool send_string_to_client( const char *line );
  virtual bool send_string_to_server( const char *line );

protected:
  jdk_linesplit server_linesplitter;
  jdk_linesplit client_linesplitter;

  const char *line_ign;
  const char *line_term;
  const char *line_term_to_send;
};



#endif
