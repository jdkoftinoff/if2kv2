#! /bin/bash

/etc/init.d/if2kd stop

rm /usr/sbin/if2kd
rm /usr/sbin/if2k_mini_compile_list
rm -r -f /etc/if2k
rm /etc/init.d/if2kd
rm /usr/sbin/if2k_uninstall.sh

echo The Internet filter is now uninstalled
