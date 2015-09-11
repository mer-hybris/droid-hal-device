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

ARCH="${ARCH:-armv7hl}"

function minfo {
    echo -e "\e[01;34m* $* \e[00m"
}

function merror {
    echo -e "\e[01;31m!! $* \e[00m"
}

function die {
    if [ -z "$*" ]; then
        merror "command failed at `date`, dying..."
    else
        merror "$*"
    fi
    exit 1
}

function die_with_log {
    if [ -f "$1" ] ; then
        tail -n10 "$1"
        minfo "Check `dirname $(pwd)`/`basename $1` for full log."
    fi
    shift
    die $*
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
}

function builddhd() {
    mb2 -t $VENDOR-$DEVICE-$ARCH -s rpm/droid-hal-$DEVICE.spec build || die

    mv -v RPMS/*$DEVICE* $LOCAL_REPO
    createrepo $LOCAL_REPO

    sb2 -t $VENDOR-$DEVICE-$ARCH -R -m sdk-install \
      ssu ar local-$DEVICE-hal file://$LOCAL_REPO

    sb2 -t $VENDOR-$DEVICE-$ARCH -R -m sdk-install \
      zypper ref
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

function yesno() {
    read -r -p "${1:-} [Y/n]" REPLY
    REPLY=${REPLY:-y}
    case $REPLY in
       [yY])
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

    [ -z "$GIT_URL" ] && die "Please give me the git URL (or directory name, if it's already installed)."


    PKG="$(basename ${GIT_URL%.git})"
    yesno "Build $PKG?"
    if [ $? == "0" ]; then
        if [ "$GIT_URL" = "$PKG" ]; then
            GIT_URL=https://github.com/mer-hybris/$PKG.git
            minfo "No git url specified, assuming $GIT_URL"
        fi

        cd "$MER_ROOT/devel/mer-hybris" || die
        LOG="`pwd`/$PKG.log"
        [ -f "$LOG" ] && rm "$LOG"

        if [ ! -d $PKG ] ; then
            minfo "Source code directory doesn't exist, cloning repository"
            git clone $GIT_URL >>$LOG 2>&1|| die_with_log "$LOG" "cloning of $GIT_URL failed"
        fi

        pushd $PKG || die
        minfo "pulling updates..."
        git pull >>$LOG 2>&1|| die_with_log "$LOG" "pulling of updates failed"
        git submodule update >>$LOG 2>&1|| die_with_log "$LOG" "pulling of updates failed"

        SPECS="$*"
        if [ -z "$SPECS" ]; then
            minfo "No spec files for package building specified, building all I can find."
            SPECS="rpm/*.spec"
        fi

        for SPEC in $SPECS ; do
            minfo "Building $SPEC"
            mb2 -s $SPEC -t $VENDOR-$DEVICE-$ARCH build >>$LOG 2>&1|| die_with_log "$LOG" "building of package failed"
        done
        minfo "Building successful, adding packages to repo"
        mkdir -p "$ANDROID_ROOT/droid-local-repo/$DEVICE/$PKG" >>$LOG 2>&1|| die_with_log "$LOG"
        rm -f "$ANDROID_ROOT/droid-local-repo/$DEVICE/$PKG/"*.rpm >>$LOG 2>&1|| die_with_log "$LOG"
        mv RPMS/*.rpm "$ANDROID_ROOT/droid-local-repo/$DEVICE/$PKG" >>$LOG 2>&1|| die_with_log "$LOG"
        createrepo "$ANDROID_ROOT/droid-local-repo/$DEVICE" >>$LOG 2>&1|| die_with_log "$LOG" "can't create repo"
        sb2 -t $VENDOR-$DEVICE-$ARCH -R -msdk-install zypper ref >>$LOG 2>&1|| die_with_log "$LOG" "can't update pkg info"
        minfo "Building of $PKG finished successfully"
        popd
    fi
    echo
}
function buildmwb {

    GIT_URL="$1"
    shift
    GIT_BRANCH="$1"
    shift

    [ -z "$GIT_URL" ] && die "Please give me the git URL (or directory name, if it's already installed)."


    PKG="$(basename ${GIT_URL%.git})"
    yesno "Build $PKG?"
    if [ $? == "0" ]; then
        if [ "$GIT_URL" = "$PKG" ]; then
            GIT_URL=https://github.com/mer-hybris/$PKG.git
            minfo "No git url specified, assuming $GIT_URL"
        fi

        cd "$MER_ROOT/devel/mer-hybris" || die
        LOG="`pwd`/$PKG.log"
        [ -f "$LOG" ] && rm "$LOG"

        if [ ! -d $PKG ] ; then
            minfo "Source code directory doesn't exist, cloning repository"
            git clone $GIT_URL -b $GIT_BRANCH>>$LOG 2>&1|| die_with_log "$LOG" "cloning of $GIT_URL failed"
        fi

        pushd $PKG || die
        minfo "pulling updates..."
        git pull >>$LOG 2>&1|| die_with_log "$LOG" "pulling of updates failed"
        git submodule update >>$LOG 2>&1|| die_with_log "$LOG" "pulling of updates failed"

        SPECS="$*"
        if [ -z "$SPECS" ]; then
            minfo "No spec files for package building specified, building all I can find."
            SPECS="rpm/*.spec"
        fi

        for SPEC in $SPECS ; do
            minfo "Building $SPEC"
            mb2 -s $SPEC -t $VENDOR-$DEVICE-$ARCH build >>$LOG 2>&1|| die_with_log "$LOG" "building of package failed"
        done
        minfo "Building successful, adding packages to repo"
        mkdir -p "$ANDROID_ROOT/droid-local-repo/$DEVICE/$PKG" >>$LOG 2>&1|| die_with_log "$LOG"
        rm -f "$ANDROID_ROOT/droid-local-repo/$DEVICE/$PKG/"*.rpm >>$LOG 2>&1|| die_with_log "$LOG"
        mv RPMS/*.rpm "$ANDROID_ROOT/droid-local-repo/$DEVICE/$PKG" >>$LOG 2>&1|| die_with_log "$LOG"
        createrepo "$ANDROID_ROOT/droid-local-repo/$DEVICE" >>$LOG 2>&1|| die_with_log "$LOG" "can't create repo"
        sb2 -t $VENDOR-$DEVICE-$ARCH -R -msdk-install zypper ref >>$LOG 2>&1|| die_with_log "$LOG" "can't update pkg info"
        minfo "Building of $PKG finished successfully"
        popd
    fi
    echo
}
