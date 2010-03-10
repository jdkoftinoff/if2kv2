#! /bin/bash

echo 'log.detail="8"' >>/Library/If2k/install.txt
rm /var/log/if2kd.log
launchctl stop com.internetfilter.if2kd
launchctl start com.internetfilter.if2kd

