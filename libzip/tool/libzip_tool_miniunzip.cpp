
#include "zlib.h"

extern "C" int unzip_main( int argc, char **argv );

int main( int argc, char **argv )
{
  return unzip_main(argc,argv);
}

