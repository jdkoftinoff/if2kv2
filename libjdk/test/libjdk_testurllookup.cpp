
#include "jdk_world.h"
#include "jdk_fast_url_lookup.h"

int main( int argc, char **argv )
{
  if( argc>2 )
  {
    jdk_str<1024> filename = argv[1];
    jdk_fast_url_lookup looker( filename );
    
    looker.save(stdout);
    jdk_fast_url_lookup::short_url_t testurl = argv[2];
    
    printf( "complete match result=%d\n", looker.find_complete_url(testurl) );
    printf( "partial match result=%d\n", looker.find_partial_url(testurl) );
    
    return 0;
  }
  return 1;
}

