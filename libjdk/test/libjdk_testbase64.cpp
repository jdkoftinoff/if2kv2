#include "jdk_world.h"
#include <iostream>
#include <cstring>
#include "jdk_base64.h"


int main( int argc, char **argv )
{
  bool encode=true;
  char inbuf[4096];
  char outbuf[4096];
  
  if( argc>1 )
  {
    encode=false;
  }

  while( std::cin.getline( inbuf, sizeof(inbuf) ) )
  {
    if( encode )
    {
      jdk_base64_encode( (void *)(&inbuf[0]), std::strlen(inbuf), outbuf, sizeof(outbuf) );
    }
    else
    {
      jdk_base64_decode( (void *)(&inbuf[0]), std::strlen(inbuf), (unsigned char  *)outbuf, sizeof(outbuf) );
    }
    std::cout << outbuf << "\n";
  }

  return 0;
}
