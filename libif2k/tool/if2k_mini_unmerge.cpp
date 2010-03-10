#include "jdk_world.h"
#include "if2k_mini_scanner.h"

static void usage()
{
  fprintf( stderr, "if2k_mini_unmerge removes matching text from stdin\nusage:\nif2k_mini_unmerge [dbfile] <inputs.txt >outputs.txt\n" );
}

int main( int argc, char **argv )
{
  if( argc!=2 )
  {
    usage();
    return 1;
  }

  const jdk_string_filename db_filename(argv[1]);

  if2k_mini_alphanumeric_scanner_precompiled scanner( db_filename );
  
  jdk_str<4096> cur_line;
  while( jdk_read_string_line( &cur_line, stdin) )
  {
    if2k_mini_tree_event<if2k_mini_alphanumeric_scanner_precompiled::tree_traits_t > ev;
    bool r=scanner.find( cur_line.c_str(), cur_line.len(), ev );
    if( r==false )
    {
      jdk_write_string_line( &cur_line, stdout );
    }
  }
}

