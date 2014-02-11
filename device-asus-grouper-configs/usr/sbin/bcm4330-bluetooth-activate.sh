#! /bin/sh
rfkill unblock bluetooth
brcm_patchram_plus --patchram /system/etc/firmware/bcm4330.hcd --no2bytes --scopcm=0,2,0,0,0,0,0,0,0,0 --enable_hci --enable_lpm --baudrate 3000000 --use_baudrate_for_download --tosleep=50000 "/dev/ttyHS2"

