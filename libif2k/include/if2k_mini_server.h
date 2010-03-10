#ifndef __IF2K_MINI_SERVER_H
#define __IF2K_MINI_SERVER_H

#include "jdk_settings.h"
#include "jdk_fork_server.h"
#include "jdk_socket.h"
#include "jdk_queue.h"
#include "jdk_linesplit.h"
#include "if2k_mini_config.h"
#include "if2k_mini_kernel.h"

class if2k_mini_server : public jdk_fork_server_child
{
  if2k_mini_server( const if2k_mini_server &o );
  const if2k_mini_server & operator = ( const if2k_mini_server &o );

public:
  explicit if2k_mini_server( const jdk_settings &settings_, if2k_mini_kernel &kernel_ )
    :
    settings( settings_ ),
    kernel( kernel_ )
    {
    }

  virtual ~if2k_mini_server() {}

  virtual void handle_sighup() { load_settings();   }

  virtual void load_settings() = 0;

  virtual bool handle_connection( jdk_client_socket &sock ) = 0;

  const jdk_settings &settings;
  if2k_mini_kernel &kernel;
};




#endif
