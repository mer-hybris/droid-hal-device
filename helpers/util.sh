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

source ~/.hadk.env

ARCH="${PORT_ARCH:-armv7hl}"
BUILDALL=n
LOG="/dev/null"

function minfo {
    echo -e "\e[01;34m* $* \e[00m"
}

function merror {
    echo -e "\e[01;31m!! $* \e[00m"
}

function die {
    if [ "$LOG" != "/dev/null" && -f "$LOG" ] ; then
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

function buildconfigs() {
    cd hybris/droid-configs
    mb2 -t $VENDOR-$DEVICE-$ARCH \
        -s rpm/droid-config-$DEVICE.spec \
         build || die
    mv -v RPMS/*.rpm $LOCAL_REPO || die
    cd ../../

    createrepo $LOCAL_REPO
    sb2 -t $VENDOR-$DEVICE-$ARCH -R -m sdk-install \
      zypper ref

    hybris/droid-configs/droid-configs-device/helpers/process_patterns.sh
}

function builddhd() {
    mb2 -t $VENDOR-$DEVICE-$ARCH -s rpm/droid-hal-$DEVICE.spec build || die

    mv -v RPMS/*$DEVICE* $LOCAL_REPO
    createrepo $LOCAL_REPO

    sb2 -t $VENDOR-$DEVICE-$ARCH -R -m sdk-install \
      ssu ar local-$DEVICE-hal file://$LOCAL_REPO

    sb2 -t $VENDOR-$DEVICE-$ARCH -R -m sdk-install \
      zypper ref

    # pickup any changes in case dhd has been rebuilt before
    sb2 -t $VENDOR-$DEVICE-$ARCH -R -m sdk-install \
      zypper dup --from local-$DEVICE-hal
}

function buildversion() {
    cd hybris/droid-hal-version-$DEVICE
    mb2 -t $VENDOR-$DEVICE-$ARCH \
      -s rpm/droid-hal-version-$DEVICE.spec \
      build || die
    mv -v RPMS/*.rpm $LOCAL_REPO
    cd ../../

    createrepo $LOCAL_REPO
    sb2 -t $VENDOR-$DEVICE-$ARCH -R -m sdk-install \
      zypper ref
}

function yesnoall() {
    if [ $BUILDALL == "y" ]; then
        return `true`
    fi
    read -r -p "${1:-} [Y/n/all]" REPLY
    REPLY=${REPLY:-y}
    case $REPLY in
       [yY])
       true
       ;;
    [aA])
       BUILDALL=y
       true
       ;;
    *)
       false
       ;;
    esac
}

function initlog {
    LOG="`pwd`/$1.log"
    [ -f "$LOG" ] && rm "$LOG"
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

        cd "$ANDROID_ROOT/hybris/mw" || die

        initlog $PKG

        if [ ! -d $PKG ] ; then
            minfo "Source code directory doesn't exist, cloning repository"
            git clone $GIT_URL $GIT_BRANCH >>$LOG 2>&1|| die "cloning of $GIT_URL failed"
        fi

        pushd $PKG > /dev/null || die
        minfo "pulling updates..."
        git pull >>$LOG 2>&1|| die "pulling of updates failed"
        git submodule update >>$LOG 2>&1|| die "pulling of updates failed"

        build $1

        deploy $PKG

        popd > /dev/null
    fi
    echo
}

# Code portions above can eventually be refactored by re-using build and deploy
# functions below

function build {
    SPECS=$1
    if [ -z "$SPECS" ]; then
        minfo "No spec file for package building specified, building all I can find."
        SPECS="rpm/*.spec"
    fi
    for SPEC in $SPECS ; do
        minfo "Building $SPEC"
        mb2 -s $SPEC -t $VENDOR-$DEVICE-$ARCH build >>$LOG 2>&1|| die "building of package failed"
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
    createrepo "$ANDROID_ROOT/droid-local-repo/$DEVICE" >>$LOG 2>&1|| die "can't create repo"
    sb2 -t $VENDOR-$DEVICE-$ARCH -R -msdk-install zypper ref >>$LOG 2>&1|| die "can't update pkg info"
    minfo "Building of $PKG finished successfully"
}

function buildpkg {
    if [ -z "$1" ]; then
        die "Please specify path to the package"
    fi
    pushd $1 > /dev/null || die "Path not found: $1"
    PKG=$(basename $1)
    initlog $PKG
    build $2
    deploy $PKG
    popd > /dev/null
}

