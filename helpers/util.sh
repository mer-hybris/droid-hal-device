#!/bin/bash
# util.sh - all refactored bits/functions go here
#
# Copyright (C) 2015 Alin Marin Elena <alin@elena.space>
# Copyright (C) 2015 Jolla Ltd.
# Contact: Simonas Leleiva <simonas.leleiva@jollamobile.com>
#
# All rights reserved.
#
# This script uses parts of code located at https://github.com/dmt4/sfa-mer
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# * Neither the name of the <organization> nor the
# names of its contributors may be used to endorse or promote products
# derived from this software without specific prior written permission.
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

PORT_ARCH="${PORT_ARCH:-armv7hl}"
BUILDALL=n
LOG="/dev/null"
CREATEREPO="createrepo_c"

function minfo {
    echo -e "\e[01;34m* $* \e[00m"
}

function merror {
    echo -e "\e[01;31m!! $* \e[00m"
}

function die {
    if [[ "$LOG" != "/dev/null" && -f "$LOG" ]] ; then
        tail -n20 "$LOG"
        minfo "Check $LOG for full log."
    fi
    if [ -z "$*" ]; then
        merror "command failed at `date`, dying..."
    else
        merror "$*"
    fi
    exit 1
}

if [ -z $DEVICE ]; then
    die 'Error: $DEVICE is undefined. Please run hadk'
fi

mkdir -p $ANDROID_ROOT/hybris/mw
zypper se -i createrepo_c > /dev/null
ret=$?
if [ $ret -eq 104 ]; then
    ANDROID_TOOLS=""
    zypper se createrepo_c > /dev/null
    ret=$?
    if [ $ret -eq 104 ]; then # SDK older than 2.2.0
        CREATEREPO="createrepo"
        zypper se -i createrepo > /dev/null
        ret=$?
        if [ $ret -eq 104 ]; then
            ANDROID_TOOLS="android-tools"
        fi
    else
        ANDROID_TOOLS="android-tools-hadk"
    fi
    if [ -n "$ANDROID_TOOLS" ]; then
        minfo Installing required Platform SDK packages
        sudo zypper in $ANDROID_TOOLS $CREATEREPO zip tar rpm-python
    fi
fi
LOCAL_REPO=$ANDROID_ROOT/droid-local-repo/$DEVICE
mkdir -p $LOCAL_REPO

function initlog {
    LOGPATH=`pwd`
    if [ -n "$2" ]; then
        LOGPATH=$2
    fi
    LOG="$LOGPATH/$1.log"
    [ -f "$LOG" ] && rm "$LOG"
}

function buildconfigs() {
    PKG=droid-configs
    cd hybris/$PKG
    initlog $PKG $(dirname `pwd`)
    build rpm/droid-config-$DEVICE.spec
    deploy $PKG do_not_install
    # installroot no longer exists since Platform SDK 2.2.0, let's put KS back
    rm -rf installroot
    mkdir installroot
    cd installroot
    rpm2cpio $ANDROID_ROOT/droid-local-repo/$DEVICE/droid-configs/droid-config-$DEVICE-ssu-kickstarts-1-1.armv7hl.rpm | cpio -idv &> /dev/null
    cd ../../../

    hybris/droid-configs/droid-configs-device/helpers/process_patterns.sh >>$LOG 2>&1|| die "error while processing patterns"
}

function builddhd() {
    PKG=droid-hal-$DEVICE
    initlog $PKG
    build rpm/$PKG.spec
    deploy $PKG do_not_install
}

function buildversion() {
    PKG=droid-hal-version-$DEVICE
    dir=$(dirname $(find hybris -name $PKG.spec))
    cd $dir/..
    initlog $PKG $(dirname `pwd`)
    build rpm/$PKG.spec
    deploy $PKG do_not_install
    cd ../../
}

function yesnoall() {
    if [ $BUILDALL == "y" ]; then
        return `true`
    fi
    read -r -p "${1:-} [Y/n/all]" REPLY
    REPLY=${REPLY:-y}
    case $REPLY in
    [yY]*)
       true
       ;;
    [aA]*)
       BUILDALL=y
       true
       ;;
    *)
       false
       ;;
    esac
}

function buildmw {

    GIT_URL="$1"
    shift
    GIT_BRANCH=""
    if [[ "$1" != "" && "$1" != *.spec ]]; then
        GIT_BRANCH="-b $1"
        shift;
    fi

    [ -z "$GIT_URL" ] && die "Please give me the git URL (or directory name, if it's already installed)."


    PKG="$(basename ${GIT_URL%.git})"
    yesnoall "Build $PKG?"
    if [ $? == "0" ]; then
        # Remove this warning when ngfd-plugin-droid-vibrator will get rid of CMake
        if [ "$GIT_URL" = "ngfd-plugin-droid-vibrator" ]; then
            merror "WARNING: ngfd-plugin-droid-vibrator build is known to halt under various scenarios!"
            merror "Please keep interrupting/rebuilding until it works. We suspect CMake and SSDs :)"
        fi

        if [ "$GIT_URL" = "$PKG" ]; then
            GIT_URL=https://github.com/mer-hybris/$PKG.git
            minfo "No git url specified, assuming $GIT_URL"
        fi

        if [ -d "$ANDROID_ROOT/external/$PKG" ]; then
            pushd "$ANDROID_ROOT/external" > /dev/null || die

            minfo "Source code directory exists in \$ANDROID_ROOT/external. Building the existing version. Make sure to update this version by updating the manifest, if required."

            initlog $PKG

            pushd $PKG > /dev/null || die
        else
            pushd "$ANDROID_ROOT/hybris/mw" > /dev/null || die

            initlog $PKG

            if [ ! -d $PKG ] ; then
                minfo "Source code directory doesn't exist, cloning repository"
                git clone --recurse-submodules $GIT_URL $GIT_BRANCH >>$LOG 2>&1|| die "cloning of $GIT_URL failed"
            fi

            pushd $PKG > /dev/null || die
            minfo "pulling updates..."
            git pull >>$LOG 2>&1|| die "pulling of updates failed"
        fi

        build $1

        deploy $PKG

        popd > /dev/null
        popd > /dev/null
    fi
}

function build {
    SPECS=$1
    if [ -z "$SPECS" ]; then
        minfo "No spec file for package building specified, building all I can find."
        SPECS="rpm/*.spec"
    fi
    for SPEC in $SPECS ; do
        minfo "Building $SPEC"
        mb2 -s $SPEC -t $VENDOR-$DEVICE-$PORT_ARCH build >>$LOG 2>&1|| die "building of package failed"
    done
}

function deploy {
    PKG=$1
    if [ -z "$PKG" ]; then
        die "Please provide a package name to build"
    fi
    minfo "Building successful, adding packages to repo"
    mkdir -p "$ANDROID_ROOT/droid-local-repo/$DEVICE/$PKG" >>$LOG 2>&1|| die
    rm -f "$ANDROID_ROOT/droid-local-repo/$DEVICE/$PKG/"*.rpm >>$LOG 2>&1|| die
    mv RPMS/*.rpm "$ANDROID_ROOT/droid-local-repo/$DEVICE/$PKG" >>$LOG 2>&1|| die "Failed to deploy the package"
    $CREATEREPO "$ANDROID_ROOT/droid-local-repo/$DEVICE" >>$LOG 2>&1|| die "can't create repo"
    sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -R -m sdk-install ssu ar local-$DEVICE-hal file://$LOCAL_REPO >>$LOG 2>&1|| die "can't add repo to target"
    sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -R -m sdk-install zypper ref || die "can't update pkg info"
    DO_NOT_INSTALL=$2
    if [ "$PKG" == "libhybris" ]; then
        # If this is the first installation of libhybris simply remove mesa,
        # otherwise proceed with force re-installation
        sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -m sdk-install -R zypper se -i mesa-llvmpipe > /dev/null
        ret=$?
        if [ $ret -eq 104 ]; then
            DO_NOT_INSTALL=
        else
            DO_NOT_INSTALL=1
            sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -R -msdk-install zypper -n rm mesa-llvmpipe >>$LOG 2>&1|| die "cannot remove mesa to get libhybris"
        fi
    fi
    if [ -z $DO_NOT_INSTALL ]; then
        # Force install due to Version unchanging in local builds,
        # and dup wouldn't work either
        # TODO: regexp match an RPM package filename to extract package name only,
        # so then it becomes possible to zypper install --force elegantly
        sb2 -t $VENDOR-$DEVICE-$PORT_ARCH -R -msdk-install zypper --non-interactive install --force --allow-unsigned-rpm $ANDROID_ROOT/droid-local-repo/$DEVICE/$PKG/*.rpm>>$LOG 2>&1|| die "can't install the package"
    fi
    minfo "Building of $PKG finished successfully"
}

function buildpkg {
    if [ -z "$1" ]; then
        die "Please specify path to the package"
    fi
    pushd $1 > /dev/null || die "Path not found: $1"
    PKG=$(basename $1)
    initlog $PKG $(dirname `pwd`)
    build $2
    deploy $PKG
    popd > /dev/null
}

