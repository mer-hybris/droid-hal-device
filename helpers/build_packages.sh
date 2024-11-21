#!/bin/bash
# build_packages.sh - takes care of rebuilding droid-hal-device, -configs, and
# -version, as well as any middleware packages. All in correct sequence, so that
# any change made could be simply picked up just by
# re-running this script.
#
# Copyright (c) 2015 Alin Marin Elena <alin@elena.space>
# Copyright (c) 2015 - 2019 Jolla Ltd.
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

usage() {
    cat <<EOF
Usage: $0 [OPTION]..."
   -h, --help      you're reading it
   -d, --droid-hal build droid-hal-device (rpm/)
   -c, --configs   build droid-configs
   -m, --mw[=REPO] build HW middleware packages or REPO
   -g, --gg        build droidmedia and supporting packages
   -v, --version   build droid-hal-version
   -i, --mic       build image
   -b, --build=PKG build one package (PKG can include path)
   -r, --revision=REV optionally used with -m
                   build package version from a specific git tag or commit hash
   -s, --spec=SPEC optionally used with -m or -b
                   can be supplied multiple times to build multiple .spec files at once
   -N  --no-auto-version
                   Tell mb2 to not fix the version inside a spec file
   -o, --offline   build offline after all repos have been cloned or refreshed
   -n, --no-delete do not delete existing packages when adding to repo

 No options assumes building for all areas.
EOF
    exit 1
}

OPTIONS=$(getopt -o hdcm::gvib:r:s:DonN -l help,droid-hal,configs,mw::,gg,version,mic,build:,revision:,spec:,do-not-install,offline,no-delete,no-auto-version -- "$@")

if [ $? -ne 0 ]; then
    echo "getopt error"
    exit 1
fi

# build all if none or only --offline parameter is in the cmdline
if [[ $# -lt 2 && "$1" =~ ^(|-o|--offline)$ ]]; then
    BUILDDHD=1
    BUILDCONFIGS=1
    BUILDMW=1
    BUILDGG=1
    BUILDVERSION=1
    BUILDIMAGE=1
fi

eval set -- $OPTIONS

BUILDSPEC_FILE=()
while true; do
    case "$1" in
      -h|--help) usage ;;
      -d|--droid-hal) BUILDDHD=1 ;;
      -c|--configs) BUILDCONFIGS=1 ;;
      -D|--do-not-install)
          echo "The '$1' option is not needed anymore and is currently ignored" >&2;;
      -N|--no-auto-version) NO_AUTO_VERSION=--no-fix-version ;;
      -m|--mw) BUILDMW=1
          BUILDMW_ASK=1
          case "$2" in
              *) BUILDMW_REPO=$2;;
          esac
          shift;;
      -g|--gg) BUILDGG=1 ;;
      -b|--build) BUILDPKG=1
          case "$2" in
              *) BUILDPKG_PATH=$2;;
          esac
          shift;;
      -r|--revision) BUILDREV=1
          case "$2" in
              *) BUILDREV_ARGS="-r $2";;
          esac
          shift;;
      -s|--spec) BUILDSPEC=1
          case "$2" in
              *) BUILDSPEC_FILE+=("$2");;
          esac
          shift;;
      -v|--version) BUILDVERSION=1 ;;
      -i|--mic) BUILDIMAGE=1 ;;
      -o|--offline) BUILDOFFLINE=1 ;;
      -n|--no-delete) NODELETE=1 ;;
      --)        shift ; break ;;
      *)         echo "unknown option: $1" ; exit 1 ;;
    esac
    shift
done

if [ "$PORT_ARCH" = "aarch64" ]; then
    _LIB=lib64
else
    _LIB=lib
fi

if [ $# -ne 0 ]; then
    echo "unknown option(s): $@"
    exit 1
fi

if [ ! -d rpm/dhd ]; then
    echo $0: 'launch this script from the $ANDROID_ROOT directory'
    exit 1
fi
# utilities
. ./rpm/dhd/helpers/util.sh

if [ "$BUILDDHD" = "1" ]; then
    builddhd
fi

if [ "$BUILDCONFIGS" = "1" -o "$BUILDIMAGE" = "1" ]; then
    if [ -n "$(grep '%define community_adaptation' $ANDROID_ROOT/hybris/droid-configs/rpm/droid-config-$DEVICE.spec)" ]; then
        community_adaptation=1
    else
        community_adaptation=0
    fi
fi

if [ "$BUILDCONFIGS" = "1" ]; then
    if [ "$community_adaptation" == "1" ]; then
        if [ "$(ls -A "$ANDROID_ROOT"/hybris/droid-configs/sparse/usr/share/ssu/repos.d 2> /dev/null)" ]; then
            build_community="localbuild-ota"
        else
            build_community="localbuild"
        fi
        sdk-assistant maintain $VENDOR-$DEVICE-$PORT_ARCH zypper se -i community-adaptation > /dev/null
        ret=$?
        if [ $ret -eq 0 ]; then
            if [ "$build_community" == "localbuild-ota" ]; then
                sdk-assistant maintain $VENDOR-$DEVICE-$PORT_ARCH zypper se -i community-adaptation-localbuild > /dev/null
                ret=$?
                if [ $ret -eq 104 ]; then
                    # Do nothing, because either localbuild-ota is already installed
                    # or user chose to keep another flavour, respect their choice.
                    build_community=
                elif [ $ret -ne 0 ]; then
                    fail=1
                fi
            fi
        elif [ $ret -ne 104 ]; then
            fail=1
        fi
        if [ $fail ]; then
            die "Could not determine if community-adaptation package is available, exiting."
        fi
        if [ -n "$build_community" ]; then
            BUILDMW_QUIET=1
            buildmw -u "https://github.com/mer-hybris/community-adaptation.git" \
                    -s rpm/community-adaptation-"$build_community".spec || die
            BUILDMW_QUIET=
        fi
    fi
    # avoid a SIGSEGV on exit of libhybris client
    sdk-assistant maintain $VENDOR-$DEVICE-$PORT_ARCH ls /system/build.prop &> /dev/null
    ret=$?
    if [ $ret -ne 0 ]; then
        sdk-assistant maintain $VENDOR-$DEVICE-$PORT_ARCH bash -c "mkdir -p /system; echo ro.build.version.sdk=99 > /system/build.prop"
    fi
    buildconfigs
    if grep -qsE "^(-|Requires:) droid-config-$DEVICE-bluez5" hybris/droid-configs/patterns/*.inc; then
        sdk-assistant maintain $VENDOR-$DEVICE-$PORT_ARCH zypper -n $PLUS_LOCAL_REPO install droid-config-$DEVICE-bluez5
    fi
fi

if [ "$BUILDMW" = "1" ]; then
    if ! sdk-assistant maintain $VENDOR-$DEVICE-$PORT_ARCH ssu s 2>/dev/null | grep -q "Device registration status: registered"; then
        sdk-assistant maintain $VENDOR-$DEVICE-$PORT_ARCH ssu domain sales
    fi
    sdk-assistant maintain $VENDOR-$DEVICE-$PORT_ARCH ssu dr sdk

    if [ "$BUILDOFFLINE" != "1" ]; then
        sdk-assistant maintain $VENDOR-$DEVICE-$PORT_ARCH zypper ref
    fi

    if [ "$FAMILY" == "" ]; then
        sdk-assistant maintain $VENDOR-$DEVICE-$PORT_ARCH zypper -n $PLUS_LOCAL_REPO install --allow-unsigned-rpm droid-hal-$DEVICE-devel
    else
        sdk-assistant maintain $VENDOR-$DEVICE-$PORT_ARCH zypper -n $PLUS_LOCAL_REPO install --allow-unsigned-rpm droid-hal-$HABUILD_DEVICE-devel
    fi

    if [ "$(sdk-assistant maintain $VENDOR-$DEVICE-$PORT_ARCH ls -A /usr/include/droid-devel/droid-headers/android-version.h 2> /dev/null)" ]; then
        android_version_header=/usr/include/droid-devel/droid-headers/android-version.h
    else
        android_version_header=/usr/$_LIB/droid-devel/droid-headers/android-version.h
    fi
    android_version_major=$(sdk-assistant maintain $VENDOR-$DEVICE-$PORT_ARCH cat $android_version_header 2>/dev/null |grep "#define.*ANDROID_VERSION_MAJOR" |sed -e "s/#define.*ANDROID_VERSION_MAJOR//g")

    manifest_lookup=$ANDROID_ROOT
    while true; do
        manifest=$manifest_lookup/.repo/manifest.xml
        if [ -f "$manifest" ] || [ "$manifest_lookup" = "$(dirname "$manifest_lookup")" ]; then
            break
        fi
        manifest=""
        manifest_lookup=$(dirname "$manifest_lookup")
    done

    if [ ! "$BUILDMW_REPO" = "" ]; then
        # No point in asking when only one mw package is being built
        BUILDMW_QUIET=1
        if [ -z "$BUILDSPEC_FILE" ]; then
            buildmw -u "$BUILDMW_REPO" $BUILDREV_ARGS || die
        else
            # Supply all given spec files from $BUILDSPEC_FILE array prefixed with "-s"
            buildmw -u "$BUILDMW_REPO" "${BUILDSPEC_FILE[@]/#/-s }" $BUILDREV_ARGS || die
        fi
    elif [ -n "$manifest" ] &&
         grep -ql hybris/mw $manifest; then
        buildmw_cmds=()
        bifs=$IFS
        while IFS= read -r line; do
            if [[ $line = *"hybris/mw"* ]]; then
                IFS="= "$'\t'
                for tok in $line; do
                    word=$(echo "$tok" | cut -d \" -f2 | cut -d \' -f2)
                    if [ "$preword" = "path" ]; then
                        if [ "$(basename $(dirname "$word"))" = "mw" ]; then
                            # Only build first level projects
                            mw=$(basename "$word")
                        fi
                    elif [ "$preword" = "spec" ]; then
                        spec=$word
                    fi
                    preword=$word
                done
                if [ ! -z "$mw" ] \
                   && [ ! "$mw" = "pulseaudio-modules-droid-glue" ]; then
                    if [ -z "$spec" ]; then
                        buildmw_cmds+=("$mw")
                    else
                        buildmw_cmds+=("$mw:$spec")
                        spec=
                    fi
                    mw=
                fi
            fi
        done < "$manifest"
        IFS=$bifs
        for bcmd in "${buildmw_cmds[@]}"; do
            bcmdsplit=(); while read -rd:; do bcmdsplit+=("$REPLY"); done <<< "$bcmd:"
            if [ ! -z "${bcmdsplit[1]}" ]; then
                buildmw -u "${bcmdsplit[0]}" -s "${bcmdsplit[1]}" || die
            elif [ ! -z "${bcmdsplit[0]}" ]; then
                buildmw -u "${bcmdsplit[0]}" || die
            fi
        done
    else
        buildmw -u "https://github.com/mer-hybris/libhybris" || die

        if [ $android_version_major -le 10 ]; then
            buildmw -u "https://github.com/mer-hybris/pulseaudio-modules-droid-jb2q.git" \
                    -s rpm/pulseaudio-modules-droid-jb2q.spec || die
        else
            buildmw -u "https://github.com/mer-hybris/pulseaudio-modules-droid.git" || die
        fi
        buildmw -u "https://github.com/mer-hybris/pulseaudio-modules-droid-hidl.git" || die
        buildmw -u "https://github.com/mer-hybris/mce-plugin-libhybris" || die
        buildmw -u "https://github.com/mer-hybris/qt5-qpa-hwcomposer-plugin" || die
        if [ $android_version_major -le 8 ]; then
            buildmw -u "https://github.com/sailfishos/sensorfw.git" \
                    -s rpm/sensorfw-qt5-hybris.spec || die
        fi
        if [ $android_version_major -le 7 ]; then
            buildmw -u "https://github.com/mer-hybris/geoclue-providers-hybris" \
                    -s rpm/geoclue-providers-hybris.spec || die
        fi
    fi
fi

if [ "$BUILDGG" = "1" ]; then

    # look for either DEVICE or HABUILD_DEVICE files, do not use wildcards as there could be other variants
    pattern_lookup=$(ls "$ANDROID_ROOT"/hybris/droid-configs/patterns/patterns-sailfish-device-adaptation-{$DEVICE,$HABUILD_DEVICE}.inc 2>/dev/null | uniq)

    if grep -qs "^Requires: gstreamer1.0-droid" "$pattern_lookup"; then
        pkg=droidmedia
        cd external/$pkg || die "Could not change directory to external/$pkg"
        droidmedia_version=$(get_package_version "$pkg")
        if [ -z "$droidmedia_version" ]; then
            # Could not obtain version, function call will have shown the error
            exit 1
        fi
        cd ../..
        rpm/dhd/helpers/pack_source_droidmedia-localbuild.sh "$droidmedia_version" ||
            die "Failed to pack_source_droidmedia-localbuild.sh"
        mkdir -p hybris/mw/droidmedia-localbuild/rpm
        cp rpm/dhd/helpers/droidmedia-localbuild.spec hybris/mw/droidmedia-localbuild/rpm/droidmedia.spec
        sed -ie "s/0.0.0/$droidmedia_version/" hybris/mw/droidmedia-localbuild/rpm/droidmedia.spec
        sed -ie "s/@PORT_ARCH@/$PORT_ARCH/" hybris/mw/droidmedia-localbuild/rpm/droidmedia.spec
        sed -ie "s/@DEVICE@/$HABUILD_DEVICE/" hybris/mw/droidmedia-localbuild/rpm/droidmedia.spec
        mv hybris/mw/droidmedia-"$droidmedia_version".tgz hybris/mw/droidmedia-localbuild
        buildmw -Nu "droidmedia-localbuild" || die
    else
        minfo "gstreamer1.0-droid not in patterns: not building droidmedia. Camera and app video playback will not be available"
    fi

    if grep -qs "^Requires: pulseaudio-modules-droid-hidl" "$pattern_lookup"; then
        minfo "Not building audioflingerglue and pulseaudio-modules-droid-glue due to pulseaudio-modules-droid-hidl in patterns"
    elif grep -qs "Requires: pulseaudio-modules-droid-glue" "$pattern_lookup"; then
        pkg=audioflingerglue
        cd external/$pkg || die "Could not change directory to external/$pkg"
        audioflingerglue_version=$(get_package_version "$pkg")
        if [ -z "$audioflingerglue_version" ]; then
            # Could not obtain version, function call will have shown the error
            exit 1
        fi
        cd ../..
        rpm/dhd/helpers/pack_source_audioflingerglue-localbuild.sh "$audioflingerglue_version" ||
            die "Failed to pack_source_audioflingerglue-localbuild.sh"
        mkdir -p hybris/mw/audioflingerglue-localbuild/rpm
        cp rpm/dhd/helpers/audioflingerglue-localbuild.spec hybris/mw/audioflingerglue-localbuild/rpm/audioflingerglue.spec
        sed -ie "s/0.0.0/$audioflingerglue_version/" hybris/mw/audioflingerglue-localbuild/rpm/audioflingerglue.spec
        sed -ie "s/@PORT_ARCH@/$PORT_ARCH/" hybris/mw/audioflingerglue-localbuild/rpm/audioflingerglue.spec
        sed -ie "s/@DEVICE@/$HABUILD_DEVICE/" hybris/mw/audioflingerglue-localbuild/rpm/audioflingerglue.spec
        mv hybris/mw/audioflingerglue-"$audioflingerglue_version".tgz hybris/mw/audioflingerglue-localbuild
        buildmw -Nu "audioflingerglue-localbuild" || die
        buildmw -u "https://github.com/mer-hybris/pulseaudio-modules-droid-glue.git" || die
    else
        minfo "Not building audioflingerglue and pulseaudio-modules-droid-glue due to the latter not being in patterns"
    fi
fi

if [ "$BUILDVERSION" = "1" ]; then
    buildversion
    echo "----------------------DONE! Now proceed on creating the rootfs------------------"
fi

if [ "$BUILDIMAGE" = "1" ]; then
    srcks="$ANDROID_ROOT/hybris/droid-configs/installroot/usr/share/kickstarts"
    ks="Jolla-@RELEASE@-$DEVICE-@ARCH@.ks"
    if sdk-assistant maintain $VENDOR-$DEVICE-$PORT_ARCH ssu s 2>/dev/null | grep -q "Release (rnd): live (devel)"; then
        bleeding_edge_build_by_sailors=1
    fi

    # Use custom location so that `zypper --plus-repo` does not pick it up - we
    # want zypper to always rescan the directory instead of forcing us to keep
    # the repodata up to date.
    mkdir -p $LOCAL_REPO/repo
    createrepo_c --outputdir=$LOCAL_REPO/repo --location-prefix=../ $LOCAL_REPO || die "can't create repo"

    if [ "$bleeding_edge_build_by_sailors" == "1" ]; then
        ks="Jolla-@RELEASE@-@FLAVOUR@-$DEVICE-@ARCH@.ks"
        ha_repo="repo --name=adaptation0-$DEVICE-@RELEASE@-@FLAVOUR@"
        if grep -q "$ha_repo" "$srcks/$ks"; then
            sed -e "s|^$ha_repo.*$|$ha_repo --baseurl=file://$LOCAL_REPO/repo|" \
                "$srcks/$ks" > $ks
        else
            # Adaptation doesn't have its repo yet
            repo_marker="repo --name=apps-@RELEASE@-@FLAVOUR@"
            sed "/$repo_marker/i$ha_repo --baseurl=file://$LOCAL_REPO/repo" \
                "$srcks/$ks" > "$ks"
        fi
    elif [ "$community_adaptation" == "1" ]; then
        ha_repo="repo --name=adaptation-community-common-$DEVICE-@RELEASE@"
        ha_dev="repo --name=adaptation-community-$DEVICE-@RELEASE@"
        if ! grep -q "$ha_repo" "$srcks/$ks"; then
            # aarch64 ports have no community-common repo for now
            ha_repo="repo --name=apps-@RELEASE@"
        fi
        sed "/$ha_repo/i$ha_dev --baseurl=file://$LOCAL_REPO/repo" \
            "$srcks/$ks" > "$ks"
        community_build="community"
    else
        ha_repo="repo --name=adaptation0-$DEVICE-@RELEASE@"
        sed -e "s|^$ha_repo.*$|$ha_repo --baseurl=file://$LOCAL_REPO/repo|" \
            "$srcks/$ks" > "$ks"
    fi
    if [ "$bleeding_edge_build_by_sailors" == "1" ]; then
        tokenmap="ARCH:$PORT_ARCH,RELEASE:live,EXTRA_NAME:$EXTRA_NAME,FLAVOUR:devel"
        flavour=devel
    else
        tokenmap="ARCH:$PORT_ARCH,RELEASE:$RELEASE,EXTRA_NAME:$EXTRA_NAME"
        flavour=release
        # Clear out extra store repositories from kickstart if exist
        sed -i "/store-repository.jolla.com/d" "$ks"
        [ -n "$RELEASE" ] || die 'Please set the desired RELEASE variable in ~/.hadk.env to build an image for'
    fi
    if [ -n $RELEASE ]; then
        release_version="-"$RELEASE
    fi
    imgname=SailfishOS"$community_build"-$flavour"$release_version"-$DEVICE"$EXTRA_NAME"
    # Check if we need to build loop or fs image
    pattern_lookup=$(ls "$ANDROID_ROOT"/hybris/droid-configs/patterns/patterns-sailfish-device-adaptation-{$DEVICE,$HABUILD_DEVICE}.inc 2>/dev/null | uniq)

    if grep -qsE "^Requires: droid-hal-($DEVICE|$HABUILD_DEVICE)-kernel-modules" "$pattern_lookup"; then
        sudo mic create fs --arch=$PORT_ARCH \
            --tokenmap=$tokenmap \
            --record-pkgs=name,url \
            --outdir=$imgname \
            --pack-to=sfe-$DEVICE-$RELEASE"$EXTRA_NAME".tar.bz2 \
            "$ANDROID_ROOT"/$ks
    else
        sudo mic create loop --arch=$PORT_ARCH \
            --tokenmap=$tokenmap \
            --record-pkgs=name,url \
            --outdir=$imgname \
            --copy-kernel \
            "$ANDROID_ROOT"/$ks
    fi
fi

if [ "$BUILDPKG" = "1" ]; then
    if [ -z $BUILDPKG_PATH ]; then
       echo "--build requires an argument (path to package)"
    else
        buildpkg $BUILDPKG_PATH "${BUILDSPEC_FILE[@]}"
    fi
fi

