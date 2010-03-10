
#include "jdk_world.h"
#include "jdk_thread.h"

class testthread : public jdk_thread
{
public:
  testthread( int id_ ) : id( id_ ), cnt(0) {}
protected:
  void main()
  {
    while(1)
    {
      ++cnt;
      fprintf( stdout, "%d %d\n", id, cnt );
      fflush(stdout);
      sleep(1);
    }
    
  }
  
  int id;
  int cnt;	
};


int main( int argc, char **argv )
{
  int num = 2;
  
  if( argc>1 )
    num=atoi(argv[1] );
  
  for( int i=0; i<num; ++i )
  {
    testthread *a = new testthread(i);
    a->run();
  }
  
  getchar();
  
  fprintf(stdout, "exiting\n" );
  fflush(stdout);
  return 0;
}

