#include "jdk_world.h"
#include "if2k_client.h"

int main( int argc, char **argv )
{
  if2k_client_settings settings;
  if2k_client client;
  client.set_connection( 
    jdk_string_url("127.0.0.1:8000"),
    jdk_string_url(""),
    jdk_string_url("password")
    );
  
  jdk_settings_text map;
  map.load_file( stdin );
  bool r =client.apply_setting( map );
  fprintf( stderr, "apply settings returns %d\n", (int)r );
  
  return r==true ? 0: 1;
}

