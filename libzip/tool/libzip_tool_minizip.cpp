
#include "zlib.h"

extern "C" int zip_main( int argc, char **argv );

int main( int argc, char **argv )
{
  return zip_main(argc,argv);
}

