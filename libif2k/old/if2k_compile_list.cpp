#include "jdk_world.h"
#include "if2k_scanner.h"

int main( int argc, char **argv )
{
  if( argc!=4 )
  {
    fprintf( stderr, "usage:\nif2k_compile_list [codeoffset] [inputfile] [outputfile]\n" );
    return 1;
  }

  int code_offset = strtol( argv[1],0,10 );
  const char *input_filename = argv[2];
  const char *output_filename = argv[3];

  if2_scanner_tree<if2_tree_traits_url> compiled_tree;
  if2_pattern_expander_standard expander;

  jdk_dynbuf input_buf;
  input_buf.append_from_file( input_filename );
  expander.load( input_buf, compiled_tree, code_offset, jdk_str<8>("") );
  compiled_tree.make_shortcuts();

  jdk_dynbuf compressed_results;
  compiled_tree.save( compressed_results );

  compressed_results.extract_to_file( output_filename );

  return 0;
}
