#!/bin/bash
# build auxiliary packages
# Copyright (c) 2014 Jolla Ltd.
# Contact: Simonas Leleiva <simonas.leleiva@jollamobile.com>

if [ ! -d hybris ]; then
    echo $0: launch this script from the $ANDROID_ROOT directory
    exit 1
fi

if [ -z $DEVICE ]; then
    echo 'Error: $DEVICE is undefined. Please run hadk'
    exit 1
fi

if [ ! -d $ANDROID_ROOT/droid-local-repo/$DEVICE/repodata ]; then
    echo 'Device-specific repo does not exist. Refer to HADK section 7.1'
    exit 1
fi

echo "------------------------------------------------------------------------"
echo "Before running this script, ensure you have built all packages from HADK"
echo "section 13.8 or otherwise made them available via external repos! Script"
echo "will simply fail if they are missing"
echo "------------------------------------------------------------------------"

# just in case you've provided newer dhd, it needs to be dupped:
sb2 -t $VENDOR-$DEVICE-armv7hl -R -msdk-install zypper dup --from=local-$DEVICE-hal

mkdir -p $ANDROID_ROOT/droid-local-repo/$DEVICE

cd hybris/droid-hal-version
mb2 -t $VENDOR-$DEVICE-armv7hl \
    -s rpm/droid-hal-version.spec \
    build
rm -f $ANDROID_ROOT/droid-local-repo/$DEVICE/droid-hal-version*rpm
mv RPMS/droid-hal-version* $ANDROID_ROOT/droid-local-repo/$DEVICE
rm -rf RPMS
cd ../..

createrepo $ANDROID_ROOT/droid-local-repo/$DEVICE

