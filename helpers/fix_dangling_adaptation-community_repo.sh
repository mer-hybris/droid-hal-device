#!/bin/bash

if [[ ! -d rpm/dhd ]]; then
    echo $0: 'launch this script from the $ANDROID_ROOT directory'
    exit 1
fi
. ./rpm/dhd/helpers/util.sh

minfo "Removing 'adaptation-community' repo from local build target"
if [ -n "$(grep '%define community_adaptation' $ANDROID_ROOT/hybris/droid-configs/rpm/droid-config-$DEVICE.spec)" ]; then
    sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -m sdk-install -R ls /usr/share/ssu/features.d/adaptation-community.ini &> /dev/null
    ret=$?
    if [ $ret -eq 2 ]; then
        echo "adaptation-community repository has never been added yet, no need to fix anything here."
    elif [ 0 -ne $(sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -m sdk-install -R stat -c%s /usr/share/ssu/features.d/adaptation-community.ini) ]; then
        echo "adaptation-community has been found pointing to an OBS repo, let's purge it for local builds"
        sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -m sdk-install -R rpm -e --nodeps community-adaptation-devel
        BUILDALL=y
        buildmw https://github.com/mer-hybris/community-adaptation.git rpm/community-adaptation-localbuild.spec || die
        BUILDALL=n
        buildconfigs
        sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -m sdk-install -R zypper --non-interactive in --force droid-config-$DEVICE
        echo "configs rebuilt, adaptation-community has now been removed"
    else 
        echo "adaptation-community is already nonexistent, job done here"
    fi
fi

