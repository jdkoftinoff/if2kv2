#include "jdk_world.h"
#include "jdk_string.h"
#include "jdk_array.h"
#include "jdk_valarray.h"
#include "jdk_list.h"
#include "jdk_map.h"
#include "jdk_settings.h"

template <class CONTAINER>
void dump_container( const CONTAINER &c )
{
  for( int i=0; i<c.count(); ++i )
  {
    const typename CONTAINER::entry_t *s = c.get(i);
    if( s )	    
      printf( "%d = %s\n", i, s->c_str() );
    else
      printf( "%d is empty\n", i );
  }
}


template <class CONTAINER>
bool test_container( int repetitions, int sz )
{
  CONTAINER c(sz);
  
  for( int rep=0; rep<repetitions; ++rep )
  {
    int i;
    // add a bunch of elements
    for( i=0; i<sz; ++i )
    {
      jdk_str<512> s;
      s.form( "%d", i );
      
      if( c.add( new jdk_str<512>(s) )==false )
      {
        printf( "error adding item %d\n", i );
        break;
      }
    }
    
    // display them
    printf( "added %d elements\n", sz );
    dump_container( c );
    
    // now remove every second element
    
    for( i=0; i<c.count(); i+=2 )
    {
      c.remove( i );
    }
    
    
    printf( "removed %d elements\n", sz/2 );
    // display items
    dump_container( c );
    
    for( i=0; i<sz; ++i )
    {
      jdk_str<512> s;
      s.form( "A %d", i );
      
      if( c.add( new jdk_str<512>(s) )==false )
      {
        printf( "error adding item %d\n", i );
        break;
      }
    }
    
    // display them
    printf( "added %d elements\n", i );
    dump_container( c );
    
    // sort them
    c.sort();
    // display them
    printf( "sorted %d\n", i );
    dump_container( c );
    
    
    // now remove everything
    for( i=0; i<c.count(); ++i )
    {
      c.remove( i );
    }
    
    printf( "removed all elements\n");
    // display items
    dump_container( c );
    
  }
  return true;
}

template <class CONTAINER>
bool test_valcontainer( int repetitions, int sz )
{
  CONTAINER c(sz);
  
  for( int rep=0; rep<repetitions; ++rep )
  {
    int i;
    // add a bunch of elements
    for( i=0; i<sz; ++i )
    {
      jdk_str<512> s;
      s.form( "%d", i );
      
      if( c.add( typename CONTAINER::entry_t(s) )==false )
      {
        printf( "error adding item %d\n", i );
        break;
      }
    }
    
    // display them
    printf( "added %d elements\n", sz );
    dump_container( c );
    
    // now remove every second element
    
    for( i=0; i<c.count(); i+=2 )
    {
      c.remove( i );
    }
    
    
    printf( "removed %d elements\n", sz/2 );
    // display items
    dump_container( c );
    
    for( i=0; i<sz; ++i )
    {
      jdk_str<512> s;
      s.form( "A %d", i );
      
      if( c.add( typename CONTAINER::entry_t(s) )==false )
      {
        printf( "error adding item %d\n", i );
        break;
      }
    }
    
    // display them
    printf( "added %d elements\n", i );
    dump_container( c );
    
    
    // now remove everything
    for( i=0; i<c.count(); ++i )
    {
      c.remove( i );
    }
    
    printf( "removed all elements\n");
    // display items
    dump_container( c );
    
  }
  return true;
}


int main( int argc, char **argv )
{
  if( argc>2 )
  {
    int reps=atoi(argv[1]);
    int sz=atoi(argv[2] );
    printf( "jdk_array:\n" );
    test_container< jdk_array< jdk_str<512> > >(reps,sz);
    printf( "jdk_valarray:\n" );
    test_valcontainer< jdk_valarray< jdk_str<512> > >(reps,sz);
    printf( "jdk_array with jdk_dynstr:\n" );
    test_valcontainer< jdk_valarray< jdk_dynstr > >(reps, sz );
  }
  return 0;
}

