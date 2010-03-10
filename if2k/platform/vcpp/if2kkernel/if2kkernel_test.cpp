
#include <stdio.h>
#include <stdlib.h>
#include "if2kkernel.h"


int main( int argc, char **argv )
{
  If2kHandle a = if2k_scanner_init( 0, 0, 0 );
  if( a!=0 )
  {
    DWORD r=if2k_scanner_scan_url( a, "http://www.internetfilter.com/", 0 );

    if2k_scanner_free(a);
  }
  return 0;
}
