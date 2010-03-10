#include "jdk_world.h"
#if JDK_HAS_SOCKETSWITCH
#include "jdk_socketswitch.h"

jdk_socketswitch::jdk_socketswitch() : maker_map(16)
{
}

jdk_socketswitch::~jdk_socketswitch()
{
}

void jdk_socketswitch::add_socket_type(
  const jdk_string &name,
  jdk_socketswitch_maker maker_proc
  )
{
  maker_map.add( new jdk_socketswitch_pair(name, maker_proc) );
}

jdk_client_socket * jdk_socketswitch::make_socket( const jdk_string &url_ )
{
  jdk_str<64> prefix;
  jdk_str<1024> url( url_ );
  jdk_client_socket *sock = 0;
  
  url.strip_begws();
  url.strip_endws();
  if( url.extract_token( 0, &prefix, ":" )>=0 )
  {
    for( int i=0; i<maker_map.count(); ++i )
    {
      const jdk_socketswitch_pair *p = maker_map.get(i);
      
      if( p && p->key == prefix )
      {
        // found it! run the creation routine
        
        sock = p->value( url );
        
        // we are done
        break;
      }			
    }
    
  }
  return sock;
}


#endif
