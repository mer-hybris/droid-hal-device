#!/bin/bash
# droid-hal device add script
# Copyright (c) 2014 Jolla Ltd.
# Contact: Simonas Leleiva <simonas.leleiva@jollamobile.com>

if [ -z $DEVICE ]; then
    echo 'Error: $DEVICE is undefined. Please run hadk'
    exit 1
fi

CONFIG_DIR=hybris/droid-configs
ROOTFS_DIR=sparse
PATTERNS_DIR=droid-configs-device/patterns
PATTERNS_DEVICE_DIR=patterns
PATTERNS_TEMPLATES_DIR=$CONFIG_DIR/$PATTERNS_DIR/templates

if [ ! -d $PATTERNS_TEMPLATES_DIR ]; then
    echo $0: launch this script from the $ANDROID_ROOT directory
    exit 1
fi

cd $CONFIG_DIR

echo Creating the following nodes:

if [[ -e $ROOTFS_DIR && ! $1 == "-y" ]]; then
    read -p "Device $DEVICE appears to be already created. Re-generate patterns? [Y/n] " -n 1 -r
    REPLY=${REPLY:-Y}
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
else
    echo $ROOTFS_DIR/
    mkdir -p $ROOTFS_DIR
fi

echo $PATTERNS_DEVICE_DIR/

mkdir -p $PATTERNS_DEVICE_DIR

for pattern in $(find $PATTERNS_DIR/templates -name *.yaml); do
    PATTERNS_FILE=$(echo $PATTERNS_DEVICE_DIR/$(basename $pattern) | sed -e "s|@DEVICE@|$DEVICE|g")
    echo $PATTERNS_FILE
    cat <<'EOF' >$PATTERNS_FILE
# Feel free to disable non-critical HA parts during devel by commenting lines out
# Generated in hadk by executing: rpm/dhd/helpers/add_new_device.sh

EOF
    sed -e 's|@DEVICE@|'$DEVICE'|g' $pattern >>$PATTERNS_FILE
done

cd -

