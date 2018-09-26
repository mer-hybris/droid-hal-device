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

function usage() {
    echo "Usage: $0 [OPTION]..."
    echo "  -h, --help      you're reading it"
    echo "  -d, --droid-hal build droid-hal-device (rpm/)"
    echo "  -c, --configs   build droid-configs"
    echo "  -m, --mw[=REPO] build HW middleware packages or REPO"
    echo "  -v, --version   build droid-hal-version"
    echo "  -b, --build=PKG build one package (PKG can include path)"
    echo "  -s, --spec=SPEC optionally used with -m or -b"
    echo " No options assumes building for all areas."
    exit 1
}

OPTIONS=$(getopt -o hdcm::vb:s: -l help,droid-hal,configs,mw::,version,build:,spec: -- "$@")

if [ $? -ne 0 ]; then
    echo "getopt error"
    exit 1
fi

eval set -- $OPTIONS

if [ "$#" == "1" ]; then
    BUILDDHD=1
    BUILDCONFIGS=1
    BUILDMW=1
    BUILDVERSION=1
fi

while true; do
    case "$1" in
      -h|--help) usage ;;
      -d|--droid-hal) BUILDDHD=1 ;;
      -c|--configs) BUILDCONFIGS=1 ;;
      -m|--mw) BUILDMW=1
          case "$2" in
              *) BUILDMW_REPO=$2;;
          esac
          shift;;
      -b|--build) BUILDPKG=1
          case "$2" in
              *) BUILDPKG_PATH=$2;;
          esac
          shift;;
      -s|--spec) BUILDSPEC=1
          case "$2" in
              *) BUILDSPEC_FILE=$2;;
          esac
          shift;;
      -v|--version) BUILDVERSION=1 ;;
      --)        shift ; break ;;
      *)         echo "unknown option: $1" ; exit 1 ;;
    esac
    shift
done

if [ $# -ne 0 ]; then
    echo "unknown option(s): $@"
    exit 1
fi

if [[ ! -d rpm/dhd ]]; then
    echo $0: 'launch this script from the $ANDROID_ROOT directory'
    exit 1
fi
# utilities
. ./rpm/dhd/helpers/util.sh

if [ "$BUILDDHD" == "1" ]; then
builddhd
fi
if [ "$BUILDCONFIGS" == "1" ]; then
if [ -n "$(grep '%define community_adaptation' $ANDROID_ROOT/hybris/droid-configs/rpm/droid-config-$DEVICE.spec)" ]; then
    sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -m sdk-install -R zypper se -i community-adaptation > /dev/null
    ret=$?
    if [ $ret -eq 104 ]; then
        BUILDALL=y
        buildmw https://github.com/mer-hybris/community-adaptation.git rpm/community-adaptation-localbuild.spec || die
        BUILDALL=n
    elif [ $ret -ne 0 ]; then
        die "Could not determine if community-adaptation package is available, exiting."
    fi
fi
# avoid a SIGSEGV on exit of libhybris client
sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -m sdk-install -R ls /system/build.prop > /dev/null
ret=$?
if [ $ret -ne 0 ]; then
    sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -m sdk-install -R bash -c "mkdir -p /system; echo ro.build.version.sdk=99 > /system/build.prop"
fi
buildconfigs
fi

if [ "$BUILDMW" == "1" ]; then
sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -R -msdk-install ssu domain sales
sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -R -msdk-install ssu dr sdk

sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -R -msdk-install zypper ref -f
sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -R -msdk-install zypper -n install $ALLOW_UNSIGNED_RPM droid-hal-$DEVICE-devel

android_version_major=$(sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -R cat /usr/lib/droid-devel/droid-headers/android-version.h |grep "#define.*ANDROID_VERSION_MAJOR" |sed -e "s/#define.*ANDROID_VERSION_MAJOR//g")

pushd $ANDROID_ROOT/hybris/mw > /dev/null

if [ "$BUILDMW_REPO" == "" ]; then
buildmw libhybris || die
buildmw "https://github.com/mer-hybris/pulseaudio-modules-droid.git" rpm/pulseaudio-modules-droid.spec || die
buildmw "https://github.com/nemomobile/mce-plugin-libhybris.git" || die
buildmw ngfd-plugin-droid-vibrator rpm/ngfd-plugin-native-vibrator.spec || die
buildmw qt5-feedback-haptics-droid-vibrator rpm/qt5-feedback-haptics-native-vibrator.spec || die
buildmw qt5-qpa-hwcomposer-plugin || die
buildmw qt5-qpa-surfaceflinger-plugin || die
buildmw "https://git.merproject.org/mer-core/qtscenegraph-adaptation.git" rpm/qtscenegraph-adaptation-droid.spec || die
buildmw "https://git.merproject.org/mer-core/sensorfw.git" rpm/sensorfw-qt5-hybris.spec || die
if [ $android_version_major -ge 8 ]; then
buildmw geoclue-providers-hybris rpm/geoclue-providers-hybris-binder.spec || die
else
buildmw geoclue-providers-hybris rpm/geoclue-providers-hybris.spec || die
fi
# build kf5bluezqt-bluez4 if not yet provided by Sailfish OS itself
sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -m sdk-install -R zypper se kf5bluezqt-bluez4 > /dev/null
ret=$?
if [ $ret -eq 104 ]; then
    buildmw "https://git.merproject.org/mer-core/kf5bluezqt.git" rpm/kf5bluezqt-bluez4.spec || die
    # pull device's bluez4 configs correctly
    sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -m sdk-install -R zypper remove bluez-configs-mer
fi
else
buildmw $BUILDMW_REPO $BUILDSPEC_FILE || die
fi
popd > /dev/null
fi

if [ "$BUILDVERSION" == "1" ]; then
buildversion
echo "----------------------DONE! Now proceed on creating the rootfs------------------"
fi

if [ "$BUILDPKG" == "1" ]; then
    if [ -z $BUILDPKG_PATH ]; then
       echo "--build requires an argument (path to package)"
    else
        buildpkg $BUILDPKG_PATH $BUILDSPEC_FILE
    fi
fi

