#!/bin/sh
DROID_BIN="/usr/libexec/droid-hybris/system/bin"
PATCHRAM_ARGS="--patchram /system/etc/firmware/bcm4330.hcd \
               --no2bytes \
               --scopcm=0,2,0,0,0,0,0,0,0,0 \
               --enable_hci \
               --enable_lpm \
               --baudrate 3000000 \
               --use_baudrate_for_download \
               --tosleep=50000"

$DROID_BIN/rfkill unblock bluetooth
$DROID_BIN/brcm_patchram_plus $PATCHRAM_ARGS /dev/ttyHS2
