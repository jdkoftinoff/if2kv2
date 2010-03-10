#! /bin/bash

/sbin/kextunload "/System/Library/Extensions/if2kext.kext" 2>/dev/null >/dev/null

/bin/launchctl unload com.internetfilter.if2kd
/bin/launchctl stop com.internetfilter.if2kd

rm -r -f "/System/Library/Extensions/if2kext.kext"
rm -r -f "/Library/LaunchDaemons/com.internetfilter.if2kd.plist"
rm -r -f "/Library/If2k"
rm -r -f "/Library/Receipts/if2ksystem.pkg"

echo "IF2K is now uninstalled."


