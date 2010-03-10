#! /bin/bash

f="$1/Contents/Resources/script"
chown root:wheel "$f"
chmod +x "$f"

echo 'log.detail="0"' >>/Library/If2k/install.txt
launchctl stop com.internetfilter.if2kd
launchctl start com.internetfilter.if2kd

