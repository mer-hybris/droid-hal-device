#!/bin/sh
cd /
sh /usr/libexec/droid/android-permission-fixup.sh &> /dev/null
touch /dev/.coldboot_done
export LD_LIBRARY_PATH=/usr/libexec/droid-hybris/system/lib/:/vendor/lib:/system/lib

# Save systemd notify socket name to let droid-init-done.sh pick it up later
mkdir -p /run/droid-hal
echo $NOTIFY_SOCKET > /run/droid-hal/notify-socket-name

exec /sbin/droid-hal-init

