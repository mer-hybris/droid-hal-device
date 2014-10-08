#!/bin/sh
#
# Copyright (C) 2014 Jolla Ltd.
# Contact: Simonas Leleiva <simonas.leleiva@jollamobile.com>
#

# This waits for mako (LG Nexus 4) WLAN firmware to be provided by conn_init

while [ ! -e /data/misc/wifi/WCNSS_qcom_wlan_nv.bin ]; do
    sleep 1
    echo "...waiting for wlan firmware to appear: WCNSS_qcom_wlan_nv.bin"
done
while [ ! -e /data/misc/wifi/WCNSS_qcom_cfg.ini ]; do
    sleep 1
    echo "...waiting for wlan firmware to appear: WCNSS_qcom_cfg.ini"
done
echo "...waiting for services to settle"
sleep 2
/system/bin/ndc softap fwreload wlan0 AP

