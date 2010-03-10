#! /bin/bash

cp /var/log/if2kd.log ~/Desktop/if2klog.txt
gzip ~/Desktop/if2kdlog.txt
chmod a+rw ~/Desktop/if2kdlog.txt

