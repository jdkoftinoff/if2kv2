
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PRINT_AND_DO(a) do { printf("%s\n",a); system(a); } while(0)

int main( int argc, char **argv )
{
  printf( "if2k_remove_tool running\n" );
  PRINT_AND_DO( "/sbin/kextunload /System/Library/Extensions/if2kext.kext" );
  
  PRINT_AND_DO( "/bin/launchctl unload com.internetfilter.if2kd");
  PRINT_AND_DO( "/bin/launchctl stop com.internetfilter.if2kd");
  
  PRINT_AND_DO( "rm -r -f /System/Library/Extensions/if2kext.kext" );
  PRINT_AND_DO( "rm -r -f /Library/LaunchDaemons/com.internetfilter.if2kd.plist" );
  PRINT_AND_DO( "rm -r -f /Library/If2k" );
  PRINT_AND_DO( "rm -r -f /Library/Receipts/if2ksystem.pkg" );
  PRINT_AND_DO( "rm -r -f /Library/StartupItems/if2k" );
  PRINT_AND_DO( "rm -r -f /Library/StartupItems/If2k" );
  
  return 0;
}
