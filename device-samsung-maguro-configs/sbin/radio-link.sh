#!/bin/sh
mkdir -p /dev/block/platform/omap/omap_hsmmc.0/by-name
ln -s /dev/mmcblk0p9 /dev/block/platform/omap/omap_hsmmc.0/by-name/radio
