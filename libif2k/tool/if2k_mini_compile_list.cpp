#include "jdk_world.h"
#include "if2k_mini_tree.h"

static void usage()
{
  fprintf( stderr, "usage:\nif2k_mini_compile_list [url|alpha] [inputfile] [outputfile] [codeoffset] [prefix]\n" );
}

int main( int argc, char **argv )
{
  if( argc<5 )
  {
    usage();
    return 1;
  }

  const char *input_filename = argv[2];
  const char *output_filename = argv[3];
  int code_offset = strtol( argv[4],0,10 );
  jdk_str<256> prefix;
  
  if( argc==6 )
  {
    prefix.cat(argv[5]);
  }

  if( strcmp(argv[1],"url")==0 )
  {
    bool r = if2k_mini_tree_compile<if2k_mini_tree_traits_url,if2k_pattern_expander_standard>(
      input_filename,
      output_filename,
      code_offset,
      prefix
      );
    return int(!r);
  }
  else if( strcmp(argv[1],"alpha")==0 )
  {
    bool r = if2k_mini_tree_compile<if2k_mini_tree_traits_alphanumeric,if2k_pattern_expander_standard>(
      input_filename,
      output_filename,
      code_offset,
      prefix
      );
    return int(!r);
  }
  else
  {
    usage();
    return 1;
  }
}
