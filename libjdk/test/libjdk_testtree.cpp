#include "jdk_world.h"
#include "jdk_tree.h"
#include "jdk_string.h"
#include "jdk_dynbuf.h"
#include "jdk_util.h"

struct example_tree_traits
{
  typedef char value_t;
  typedef int index_t;
  typedef char flags_t;
  typedef jdk_dynbuf buf_t;
  typedef jdk_tree_case_insensitive_comparator<value_t> comparator_t;
  typedef jdk_tree_whitespace_ignorer<value_t> ignorer_t;
  typedef jdk_tree_basic_leaf< value_t, index_t, flags_t > leaf_t;
};

template <class LEAF_T>
void dumpone( const LEAF_T &i )
{
  fprintf( stdout, "value = '%c' parent=%2d child=%2d sibling=%2d flags='%c' end=%1d",
           i.value, i.parent, i.child, i.sibling, i.flags, i.is_end() );
}

template <class TREE_T>
void dump( const TREE_T &tree )
{
  for( int i=0; i<tree.count(); ++i )
  {
    if( !tree.get(i).is_free() )
    {
      fprintf( stdout, "\nindex: %2d  ", i );
      dumpone( tree.get(i) );
    }
  }
  fprintf( stdout, "\n" );
}

template <class TREE_T>
bool find_shortest( const TREE_T &tree, const typename TREE_T::value_t *s )
{
  typename TREE_T::flags_t flags;
  typename TREE_T::index_t i;
  int match_len;
  
  bool r=tree.find( s, strlen(s), &flags, &match_len, &i );
  
  if( r )
  {
    typename TREE_T::value_t match[64];
    int extract_len = tree.extract( match, 64, i );
    match[extract_len]=0;
    
    fprintf( stdout, "flag='%c', match_len=%d : Match %d '%s' : ", flags, match_len, extract_len, match );
  }
  return r;
}

template <class TREE_T>
bool find_longest( const TREE_T &tree, const typename TREE_T::value_t *s )
{
  typename TREE_T::flags_t flags;
  typename TREE_T::index_t i;
  int match_len;
  
  bool r=tree.find_longest( s, strlen(s), &flags, &match_len, &i );
  
  if( r )
  {
    typename TREE_T::value_t match[64];
    int extract_len = tree.extract( match, 64, i );
    match[extract_len]=0;
    
    fprintf( stdout, "flag='%c', match_len=%d : Match %d '%s' : ", flags, match_len, extract_len, match );
  }
  return r;
}

template <class TREE_T>
void extract_tree( const TREE_T &tree )
{	
  int num = tree.count_items_used();
  
  for( int i=0; i<num; ++i )
  {
    const typename TREE_T::leaf_t a = tree.get( i );
    typename TREE_T::value_t buf[256];
    
    if( a.is_end() )
    {
      int len = tree.extract( buf, 256, i );
      buf[len]=0;
      fprintf( stdout, "extracted: %s\n", buf );
    }		
  }
}


#define TEST(a,expected) do { bool r=a; fprintf( stdout, "\nTEST %s: %s = %d, expected %d\n", (r==(expected)) ? "SUCCESS" : "FAILED" ,#a, r, expected ); } while(0)

int main( int argc, char **argv )
{
  if( argc==1 )
  {		
    jdk_tree<example_tree_traits> tree;
    
    printf( "sizeof leaf_t=%d\n", (int)sizeof( example_tree_traits::leaf_t ) );
    
    tree.add( "JEFFREY", 7, '1' );
    tree.add( "JEFF", 4, '5' );    	
    
    tree.add( "JENNI", 5, '2' );
    tree.add( "Jeff Koftinoff", 14, '3' );
    tree.add( "SuperNova", 9, '4' );
    
    dump(tree);	
    
    TEST( find_longest( tree, "JEN" ), 0 );
    TEST( find_longest( tree, "JENNIFER" ), 1 );    
    TEST( find_longest( tree, "JEFFREY"), 1 );
    TEST( find_longest( tree, "JENNIFER" ), 1 );    
    TEST( find_longest( tree, "Jenny" ), 0 );
    TEST( find_longest( tree, "JEFFREY KOFTINOFF"), 1 );
    TEST( find_longest( tree, " JeFF  KoftiNofF" ), 1 );
    TEST( find_longest( tree, "jeremy" ), 0 );
    TEST( find_longest( tree, "bob" ), 0 );
    TEST( find_longest( tree, "jEff  \tKoft  in off   \r\n"), 1 ); 
    TEST( find_longest( tree, "jeff k"), 1 );
    TEST( find_longest( tree, "jeff"), 1 );	
    TEST( find_longest( tree, "jef" ), 0 );
    TEST( find_longest( tree, "Supernov" ), 0 );
    TEST( find_longest( tree, "Super nova" ), 1 );
    TEST( find_longest( tree, "ABCSuper nova" ), 0 );    
    
    // try copy the entire tree via the buffer.
    
    example_tree_traits::buf_t newbuf( tree.getbuf() );
    jdk_tree<example_tree_traits> newtree(newbuf);
    TEST( find_longest( tree,  "JEFFREY KOFTINOFF"), 1 );
    fprintf( stdout, "count_items() = %d\n", tree.count_items_used() );
    
    
    extract_tree( tree );
    
    size_t sz = tree.calc_memory_used();
    tree.getbuf().extract_file( "libjdk_testtree.tree1dump", 0, sz );
    
    fprintf( stdout, "saved tree to libjdk_testtree.tree1dump\n" );
    
    jdk_tree<example_tree_traits> tree2;
    tree2.getbuf().clear(0);
    if( tree2.getbuf().append_file( "libjdk_testtree.tree1dump" ) )
    {	  		
      fprintf( stdout, "loaded tree2 from libjdk_testtree.tree1dump:\n" );
      dump( tree2 );
      extract_tree( tree2 );
    }
  }
  else
  {
    jdk_tree<example_tree_traits> tree;
    
    FILE *f = fopen( argv[1], "rt" );
    if( f )  
    {
      jdk_str<256> line;
      while( jdk_read_string_line( &line, f ) )
      {
        if( line.len()>4 )
        {
          tree.add( line.c_str(), line.len(), '1' );
        }				
      }			
      fclose(f);
    }
    
    jdk_str<256> outf;
    outf.cpy( argv[1] );
    outf.cat( ".tree" );
    size_t sz = tree.calc_memory_used();
    tree.getbuf().extract_file( outf.c_str(), 0, sz );
    
    //extract_tree( tree );
  }
  
  
  return 0;
}


