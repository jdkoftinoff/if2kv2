#include "jdk_world.h"
#include "if2k_mini_config.h"
#include "if2k_mini_nntp_proxy.h"

if2k_mini_nntp_proxy::if2k_mini_nntp_proxy( const jdk_settings &settings_, if2k_mini_kernel &kernel_ )
  : 
  if2k_mini_server( settings_, kernel_ )
{
}

if2k_mini_nntp_proxy::~if2k_mini_nntp_proxy()
{
}

void if2k_mini_nntp_proxy::load_settings()
{
  nntp_server = settings.get("nntp.server");
}

bool if2k_mini_nntp_proxy::handle_connection( jdk_client_socket &incoming_socket )
{
  jdk_string_url client_addr;
  incoming_socket.get_remote_addr( client_addr );
  jdk_log( JDK_LOG_DEBUG1, "nntpfilter connection from :%s", client_addr.c_str() );

  jdk_inet_client_socket outgoing_socket;
  if( outgoing_socket.make_connection( nntp_server, 0, false ) )
  {
    jdk_log( JDK_LOG_DEBUG1, "Made connection to : %s", nntp_server.c_str() );
    incoming_socket.setup_socket_blocking( false );
    outgoing_socket.setup_socket_blocking( false );    

    if2k_mini_nntp_tunnel tunnel(incoming_socket, outgoing_socket, *this);
    tunnel.handle_transaction( 0, 0, 0, 0 );
  }
  else
  {
    jdk_log( JDK_LOG_DEBUG1, "Failed connection to : %s", nntp_server.c_str() );
  }

  return true;
}


bool if2k_mini_nntp_tunnel::handle_client_incoming_line( char *line )
{
//  jdk_log( JDK_LOG_DEBUG4, "from client:%s:", line );
	if( !posting_article )
	{
    if( !jdk_strnicmp( line, "GROUP ", 6 ) )
    {
      cur_newsgroup.cpy( line );
      jdk_log( JDK_LOG_DEBUG1, "In group: %s", line );
      newsgroup_entered=true;      
    }
    else
    if( !jdk_strnicmp( line, "ARTICLE ", 8 ) )
    {
      jdk_log( JDK_LOG_DEBUG1, "Reading article" );
      reading_article=true;
      seen_subject=false;
      seen_xover=false;      
    }
    else
    if(  !seen_xover && !jdk_strnicmp( line, "XOVER ", 6 ) )
    {
      jdk_log( JDK_LOG_DEBUG1, "In XOVER mode" );
      seen_xover=true;
      reading_article=false;
    }
    else
    if( newsgroup_entered && !jdk_strnicmp( line, "POST ", 5 ) )
    {
      jdk_log( JDK_LOG_DEBUG1, "Posting message" );
      posting_article=true;
      seen_subject=false;
      msg_subject.clear();
    }
  }
	else
	{
    // Check for transmit of single dot, ending post of article.
		// check for message subject and log it.
		if( !strcmp(line,".") )
		{
      jdk_log( JDK_LOG_DEBUG1, "End of post" );
      posting_article=false;
			seen_subject=false;
		}
		if( !seen_subject && !jdk_strnicmp(line,"Subject: ", 9 ) )
		{
      jdk_log( JDK_LOG_DEBUG1, "Seen subject: %s", line );

      seen_subject=true;
		}
  }


  return if2k_mini_line_tunnel::handle_client_incoming_line( line );
}

bool if2k_mini_nntp_tunnel::handle_server_incoming_line( char *line )
{
//  jdk_log( JDK_LOG_DEBUG4, "from server:%s:", line );
  if( reading_article && !seen_subject && !jdk_strnicmp( line, "Subject: ", 9 ) )
  {
    jdk_log( JDK_LOG_DEBUG1, "Reading subject: %s", line );
    seen_subject=true;
  }
  if( reading_article && seen_subject && !jdk_strnicmp( line, "begin ", 6 ) && (line[6]>'0' && line[6]<='9' ) )
  {
    jdk_log( JDK_LOG_DEBUG1, "Receiving UUENCODED attachment: %s", line );
  }

  return if2k_mini_line_tunnel::handle_server_incoming_line( line );
}
