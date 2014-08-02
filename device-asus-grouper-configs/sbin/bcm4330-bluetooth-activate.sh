#!/bin/sh
PATCHRAM_ARGS="--patchram /system/etc/firmware/bcm4330.hcd \
               --no2bytes \
               --scopcm=0,2,0,0,0,0,0,0,0,0 \
               --enable_hci \
               --enable_lpm \
               --baudrate 3000000 \
               --use_baudrate_for_download \
               --tosleep=50000"

if [ -f /system/bin/rfkill ]; then
    /system/bin/rfkill unblock bluetooth
else
    rfkill unblock bluetooth
fi

if [ -f /system/bin/brcm_patchram_plus ]; then
    /system/bin/brcm_patchram_plus $PATCHRAM_ARGS /dev/ttyHS2
else
    brcm_patchram_plus $PATCHRAM_ARGS /dev/ttyHS2
fi

