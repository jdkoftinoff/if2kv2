#! /bin/bash

install -m 700 usr/sbin/if2kd /usr/sbin/
install -m 755 usr/sbin/if2k_mini_compile_list /usr/sbin/
install -m 755 usr/sbin/if2k_compile_all_lists.sh /usr/sbin/
install -m 700 usr/sbin/if2k_uninstall.sh /usr/sbin/

install -m 700 etc/init.d/if2kd /etc/init.d/

mkdir /etc/if2k
mkdir /etc/if2k/predb
mkdir /etc/if2k/db
mkdir /etc/if2k/web
chmod -R 700 /etc/if2k

install -m 700 etc/if2k/*.txt /etc/if2k/
install -m 700 etc/if2k/predb/* /etc/if2k/predb/
install -m 700 etc/if2k/db/* /etc/if2k/db/
install -m 700 etc/if2k/web/* /etc/if2k/web/


if2k_compile_all_lists.sh

echo
echo The internet filter is now installed, however you must edit the file /etc/if2k/settings.txt
echo After you edit the settings, you may start it with:  
echo   /etc/init.d/if2kd start
echo 
echo and stop it with: 
echo   /etc/init.d/if2kd stop
echo
echo Use 'chkconfig if2kd' to enable change the runlevels that the filter runs at.
echo
echo Type 'if2k_uninstall.sh' to uninstall the filter. It lives in /usr/sbin/
echo
echo if2kd is enabled on the following runlevels:

chkconfig -a if2kd

