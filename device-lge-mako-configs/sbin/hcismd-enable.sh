#!/bin/sh

# Maximum number of attempts to enable hcismd to try to get
# hci0 to come online.  Writing to sysfs too early seems to
# not work, so we loop.
MAXTRIES=15

seq 1 $MAXTRIES | while read i ; do
    echo 1 > /sys/module/hci_smd/parameters/hcismd_set
    if [ -e /sys/class/bluetooth/hci0 ] ; then
        # found hci0, exit successfully
        exit 0
    fi
    sleep 1
done
# must have gotten through all our retries, fail
exit 1
