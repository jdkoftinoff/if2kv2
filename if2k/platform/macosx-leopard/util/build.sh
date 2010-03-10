#! /bin/bash

rm -r -f 'Enable If2k Debug.app'
/usr/local/bin/platypus -A -a 'Enable If2k Debug' -t 'Shell' -o 'None' -u 'Jeff Koftinoff' -i '/bin/sh' -V '1.0' -s '????' -I 'com.internetfilter.EnableIf2kDebug' -f 'enable_.sh' 'Enable If2k Debug.sh' 'Enable If2k Debug.app'

rm -r -f 'Disable If2k Debug.app'
/usr/local/bin/platypus -A -a 'Disable If2k Debug' -t 'Shell' -o 'None' -u 'Jeff Koftinoff' -i '/bin/sh' -V '1.0' -s '????' -I 'com.internetfilter.DisableIf2kDebug' -f 'disable_.sh' 'Disable If2k Debug.sh' 'Disable If2k Debug.app'

rm -r -f 'Dump If2k Debug.app'
/usr/local/bin/platypus -A -a 'Dump If2k Debug' -t 'Shell' -o 'None' -u 'Jeff Koftinoff' -i '/bin/sh' -V '1.0' -s '????' -I 'com.internetfilter.DumpIf2kDebug' -f 'dump_.sh' 'Dump If2k Debug.sh' 'Dump If2k Debug.app'
