#! /bin/bash

f="$1/Contents/Resources/script"
chown root:wheel "$f"
chmod +x "$f"
chmod +s "$f"

echo 'log.detail="8"' >>/Library/If2k/install.txt
rm /var/log/if2kd.log

killall -HUP if2kd

