#!/bin/bash
# build_packages.sh - takes care of rebuilding droid-hal-device, -configs, and
# -version, as well as any middleware packages. All in correct sequence, so that
# any change made (e.g. to patterns) could be simply picked up just by
# re-running this script.
#
# Copyright (C) 2015 Alin Marin Elena <alin@elena.space>
# Copyright (C) 2015 Jolla Ltd.
# Contact: Simonas Leleiva <simonas.leleiva@jollamobile.com>
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the <organization> nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

if [ -z $DEVICE ]; then
    echo 'Error: $DEVICE is undefined. Please run hadk'
    exit 1
fi
if [[ ! -d rpm/helpers && ! -d rpm/dhd ]]; then
    echo $0: launch this script from the $ANDROID_ROOT directory
    exit 1
fi

# utilities
. $ANDROID_ROOT/rpm/dhd/helpers/util.sh


if [ ! -d rpm/dhd ]; then
    echo "rpm/dhd/ does not exist, please run migrate first."
    exit 1
fi
LOCAL_REPO=$ANDROID_ROOT/droid-local-repo/$DEVICE
mkdir -p $LOCAL_REPO
rm -rf $LOCAL_REPO/droid-hal-*
rm -rf $LOCAL_REPO/droid-config-*
builddhd
buildconfigs
echo "-------------------------------------------------------------------------------"

read -p 'About to build HA middleware packages. Press Enter to continue.'
sb2 -t $VENDOR-$DEVICE-$ARCH -R -msdk-install ssu domain sales
sb2 -t $VENDOR-$DEVICE-$ARCH -R -msdk-install ssu dr sdk

sb2 -t $VENDOR-$DEVICE-$ARCH -R -msdk-install zypper ref -f
sb2 -t $VENDOR-$DEVICE-$ARCH -R -msdk-install zypper -n install droid-hal-$DEVICE-devel

rm -rf $MER_ROOT/devel/mer-hybris
mkdir -p $MER_ROOT/devel/mer-hybris
pushd $MER_ROOT/devel/mer-hybris

buildmw libhybris || die
sb2 -t $VENDOR-$DEVICE-$ARCH -R -msdk-install zypper -n rm mesa-llvmpipe
buildmw "https://github.com/nemomobile/mce-plugin-libhybris.git" || die
buildmw ngfd-plugin-droid-vibrator || die
buildmw "https://github.com/mer-hybris/pulseaudio-modules-droid.git" rpm/pulseaudio-modules-droid.spec || die
buildmw qt5-feedback-haptics-droid-vibrator || die
buildmwb qt5-qpa-hwcomposer-plugin qt-5.2 || die
buildmw "https://github.com/mer-hybris/qtscenegraph-adaptation.git" rpm/qtscenegraph-adaptation-droid.spec || die
buildmw "https://git.merproject.org/mer-core/sensorfw.git" rpm/sensorfw-qt5-hybris.spec || die
buildmw geoclue-providers-hybris || die
read -p '"Build HA Middleware Packages built". Press Enter to continue.'
popd

buildversion
echo "----------------------DONE! Now proceed on creating the rootfs------------------"
