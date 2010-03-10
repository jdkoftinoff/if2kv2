#include "jdk_world.h"
#include "jdk_string.h"
#include "jdk_util.h"
#include "if2k_pattern_expander.h"

class test_target : public if2_pattern_target
{
public:
  test_target( FILE *f_ ) : f(f_) {}
  
  void add( const jdk_string &s, short flags )
  {
    jdk_write_string_line( &s, f );
  }
  
  void remove( const jdk_string &s )
  {
    fprintf( f, "removing: " );
    jdk_write_string_line( &s, f );
  }
  
  FILE *f;
};

int main( int argc, char **argv )
{
  test_target target(stdout);
  if2_pattern_expander_standard expander;
  jdk_str<256> prefix;
  
  if( argc>1 )
  {
    prefix.cpy(argv[1]);	
  }
  
  
  jdk_str<4096> line;
  while( jdk_read_string_line(&line,stdin) )
  {
    if( line.len()>0 )
    {
      expander.expand( line, target, 0, false, prefix );
    }
  }
  return 0;
}
