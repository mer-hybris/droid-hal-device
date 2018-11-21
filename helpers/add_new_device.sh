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
METAPKG_DIR=droid-configs-device/metapkg
METAPKG_DEVICE_DIR=metapkg
METAPKG_TEMPLATES_DIR=$CONFIG_DIR/$METAPKG_DIR/templates

if [ ! -d $METAPKG_TEMPLATES_DIR ]; then
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

echo $METAPKG_DEVICE_DIR/

mkdir -p $METAPKG_DEVICE_DIR

for metapkg in $(find $METAPKG_DIR/templates -name *.spec); do
    METAPKG_FILE=$(echo $METAPKG_DEVICE_DIR/$(basename $metapkg) | sed -e "s|@DEVICE@|$DEVICE|g")
    echo $METAPKG_FILE
    cat <<'EOF' >$METAPKG_FILE
# Feel free to disable non-critical HA parts during devel by commenting lines out
# Generated in hadk by executing: rpm/dhd/helpers/add_new_device.sh

EOF
    sed -e 's|@DEVICE@|'$DEVICE'|g' $metapkg >>$METAPKG_FILE
done

cd -

