#ifndef __IF2K_MINI_NNTP_PROXY_H
#define __IF2K_MINI_NNTP_PROXY_H

#include "if2k_mini_config.h"
#include "jdk_linesplit.h"
#include "if2k_mini_server.h"
#include "if2k_mini_tunnel.h"

class if2k_mini_nntp_proxy : public if2k_mini_server
{
  if2k_mini_nntp_proxy( const if2k_mini_nntp_proxy &o );
  const if2k_mini_nntp_proxy & operator = ( const if2k_mini_nntp_proxy &o );

public:
  explicit if2k_mini_nntp_proxy( const jdk_settings &settings_, if2k_mini_kernel &kernel_ );
  virtual ~if2k_mini_nntp_proxy();

  virtual void load_settings();

  virtual bool handle_connection( jdk_client_socket &sock );

protected:
  jdk_string_url nntp_server;
};

class if2k_mini_nntp_tunnel : public if2k_mini_line_tunnel
{
public:
  explicit if2k_mini_nntp_tunnel( 
      jdk_client_socket &incoming_,
      jdk_client_socket &outgoing_,
      const if2k_mini_nntp_proxy &parent_ 
      )
    :
      if2k_mini_line_tunnel( incoming_, outgoing_ ),
      parent( parent_ ),
      seen_subject( false ),
      newsgroup_entered(false),
      reading_article(false),
      seen_xover(false),
      posting_article(false),
      blocking_newsgroup(false),
      blocking_article(false)
      {
      }

  virtual bool handle_client_incoming_line( char *line );

  virtual bool handle_server_incoming_line( char *line );

protected:
  const if2k_mini_nntp_proxy &parent;


  bool seen_subject;
  jdk_string_url msg_subject;

  bool newsgroup_entered;
  jdk_string_url cur_newsgroup;
  
  bool reading_article;
  bool seen_xover;
  bool posting_article;
  
  bool blocking_newsgroup;
  bool blocking_article;

  
};

#endif

