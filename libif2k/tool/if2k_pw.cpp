#include "if2k_mini_config.h"

int main( int argc, char *argv[] )
{
  jdk_str<512> p( argv[1] );
  jdk_str<512> ph;
  if2k_hash_for_password( ph, p );
  fprintf( stdout, "%s\n%s\n", p.c_str(), ph.c_str() );
  return 0;
}
