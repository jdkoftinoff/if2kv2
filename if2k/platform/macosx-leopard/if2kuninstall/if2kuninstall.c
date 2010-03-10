#include <stdio.h>

int main (int argc, const char * argv[]) 
{

  system( "/sbin/kextunload /System/Library/Extensions/if2kext.kext" );
   
  system( "/bin/launchctl unload com.internetfilter.if2kd");
  system( "/bin/launchctl stop com.internetfilter.if2kd");

  system( "rm -r -f /System/Library/Extensions/if2kext.kext" );
  system( "rm -r -f /Library/LaunchDaemons/com.internetfilter.if2kd.plist" );
  system( "rm -r -f /Library/If2k" );
  system( "rm -r -f /Library/Receipts/if2ksystem.pkg" );

  return 0;
}

