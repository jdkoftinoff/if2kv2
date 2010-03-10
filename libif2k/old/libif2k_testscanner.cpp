
#include "jdk_world.h"
#include "if2k_scanner.h"

template <class tree_traits>
class test_if2_scanner_event : public if2_scanner_event<tree_traits>
{
public:
  void operator () ( const jdk_tree<tree_traits> &tree, typename tree_traits::index_t match_item ) 
  {
    typename tree_traits::flags_t flag;
    typename tree_traits::value_t match_string[4096];
    
    match_string[ tree.extract( match_string, 4096, match_item) ] = '\0';
    flag = tree.get(match_item).flags;
    
    fprintf( stdout, "matched '%s', flag=%d\n", match_string, flag );
  }
  
};

int main( int argc, char **argv )
{
  if( argc>1 )
  {
    if2_scanner_tree<if2_tree_traits_alphanumeric> scanner;
    
    {
      jdk_dynbuf buf;
      if( !buf.append_file( argv[1] ) )
      {
        fprintf( stderr, "error opening %s\n", argv[1] );
        return 1;				
      }
      scanner.load(buf);
    }
    
    
    {
      test_if2_scanner_event<if2_tree_traits_alphanumeric> event;
      jdk_str<4096> line;
      
      while( jdk_read_string_line( &line, stdin ) )
      {
        scanner.find( line.c_str(), line.len(), event );
      }			
    }
    return 0;		
  }
  else
  {
    fprintf( stderr, "Please give me file name of tree file\n" );
    return 1;
  }
  
}

