#!/bin/sh
# create/update patterns in local repo
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

RPMPATH="$ANDROID_ROOT/droid-local-repo/$DEVICE"
RPMPATTERN='*-patterns*.rpm'
RPMFILE="$RPMPATH/$RPMPATTERN"
RPMCOUNT=$(find $RPMPATH -type f -name $RPMPATTERN | wc -l)
echo "checking for $RPMFILE..."
if [ $RPMCOUNT -gt 1 ]; then
    echo 'Error: more than one patterns RPM found. Please leave only one version'
    exit 1
elif [ $RPMCOUNT == 0 ]; then
    echo 'Error: no patterns RPM found'
    exit 1
fi

mkdir -p tmp/patterns
cd tmp/patterns
rpm2cpio $RPMFILE | cpio -uidv
COUNT=$(find . -type f -name "*.xml" | wc -l)
echo "<patterns count=\"$COUNT\">" >  ../patterns.xml
find . -type f -name "*.xml" -exec cat {} + >> ../patterns.xml
echo "</patterns>" >>  ../patterns.xml
modifyrepo ../patterns.xml $ANDROID_ROOT/droid-local-repo/$DEVICE/repodata
cd ../..
rm -rf tmp/patterns
rm tmp/patterns.xml

