#!/bin/sh
cd /
sh /usr/libexec/droid/android-permission-fixup.sh &> /dev/null
touch /dev/.coldboot_done
export LD_LIBRARY_PATH=/usr/libexec/droid-hybris/system/lib/:/vendor/lib:/system/lib
exec /sbin/droid-hal-init

