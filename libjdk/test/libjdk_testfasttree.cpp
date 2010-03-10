#include "jdk_world.h"
#include "jdk_fasttree.h"
#include "jdk_string.h"


typedef jdk_fasttree<int,32,char> my_tree;

template <class TREE>
struct my_iterator : public TREE::iterator_callback
{
  bool operator () ( typename TREE::key_t *key, int len, typename TREE::value_t value )
  {
    if( value!=0 )
    {			
      for( int i=0; i<len; ++i )
      {
        fputc( (int)key[i]+0x40, stdout );
      }
      fprintf( stdout, " = %d\n", (int)value );
    }
    return true;
  }
};


int main(int argc, char **argv )
{
  jdk_str<1024> line;
  
  my_tree tree;
  
  int item=1;
  
  while( jdk_read_string_line( &line, stdin ) )
  {
    if( line.len()==0 )
      break;
    
    tree.add( line.c_str(), line.len(), item++ );
  }		  
  
  tree.remove( "jeffrey", 7 );
  
  {		
    my_tree::key_t buf[1024];
    my_iterator<my_tree> it;
    tree.iterate( buf, 1024, it );
  }	
  
//	printf( "total tree elements used= %d, mem=%ld\n", fasttree_count, (long)sizeof(tree)*fasttree_count );
  
  while( jdk_read_string_line( &line, stdin ) )
  {
    if( line.len()==0 )
      break;
    
    int r = tree.find(line.c_str(),line.len());
    printf( "%s = %d\n", line.c_str(), (int)r );
  }
  
  return 0;
}
