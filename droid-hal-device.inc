# This file should be %%included into a device specific spec file
# where macros are defined:
#
# Device information:
# device:            should be the CM codename or the AOSP TARGET_PRODUCT
# vendor:            determine the droid-side directory used for
#                      ./device/<vendor>/<device>
# rpm_device:        device name used rpm-side (eg in configs)
#                      defaults to device
# rpm_vendor:        vendor name used rpm-side (eg in configs)
#                      defaults to vendor
# droid_target_aarch64:
#                    indicates that your device is 64bit ARM and has such
#                      Android base
# droid_target_armv7hl:
#                    indicates that your device is 32bit ARM. Only to be used
#                    with Android 6+ trees, where 64bit architecture is also
#                      available
#
# Device capabilities:
# installable_zip:   adds zip updater scripts (default for all SFE devices)
# enable_kernel_update:
#                    ability to flash kernel partition, inspect the usage of
#                      this macro
# enable_bootloader_update:
#                    ability to flash aboot partition, inspect the usage of this
#                      macro
#
# Miscellaneous:
# hadk_make_target:  the target used when running make in the HABUILD_SDK on the
#                      OBS. Defaults to "hybris-hal"
# device_target_cpu: used for native builds. Normally the nested droid build env
#                      will do an old-fashioned cross-compile and produce
#                      non-x86 binaries (default armv7hl). This can be set to
#                      tell OBS what arch the binaries are. E.g. Android for
#                      Intel arch must set this
# device_variant:    for AOSP this is used as the TARGET_BUILD_VARIANT for lunch
# lunch_device:      cases where the lunch combo is different from device name.
#                      For example, it's "aosp_f5121" for the "suzu" device
# makefstab_skip_entries:
#                    allow entries into the makefstab unit creation to be
#                      skipped
# have_custom_img_boot:
#                    defined when img-boot is packaged separately from
#                      droid-hal-device
# have_custom_img_recovery:
#                    defined when img-recovery is packaged separately
#                      from droid-hal-device
# bootloader_binary: define bootloader binary name when aboot is packaged
#                      separately from droid-hal-device
# pre_actions:       command to execute on OBS before `. build/envsetup.sh`,
#                      such as choosing correct Java VM
# provides_own_headers:
#                    extract-headers.sh script is not run, but android-config.h
#                      is generated
# header_patches:    if header_patches directory exists apply *.patch files to
#                      extracted headers. If provides_own_headers is defined it
#                      disables patch applying
# have_vendor_src_for_obs:
#                    include a separately packaged vendor source for OBS builds
#
# Multi-line macros:
# Such macros are defined in the .spec file as follows:

%define example_multiline_macro\
line1\
line2\
such_macro supports spaces\
#define also_supports "C-style definitions"\
%{nil}

# Available multi-line macros:
# android_config:    any custom defines you would like to pass to the underlying
#                      Android. Available options for e.g. libhybris:
#   #define WANT_ADRENO_QUIRKS 1
#                    use this if SFOS Browser has shaders crash on a 64bit device
#   #define MALI_QUIRKS 1
#                    use this if your device has a Mali GPU
# straggler_files:   any files that don't get packaged automatically, also needs
#                      droid-hal-$DEVICE-detritus in patterns


# Set defaults if not defined already:
%if 0%{!?rpm_device:1}
%define rpm_device %{device}
%endif
%if 0%{!?rpm_vendor:1}
%define rpm_vendor %{vendor}
%endif

%define __requires_exclude ^.*$
%define __provides_exclude_from ^%{_libexecdir}/droid-hybris/.*$
%define android_root .
%define dhd_path rpm/dhd

# On the OBS this package should be built in the i486 scheduler against
# mer/sailfish *_i486 targets.
# The prjconf should have an ExportFilter like this (mer/sailfish has this):
#   ExportFilter: \.armv7hl\.rpm$ armv8el
# We lie about our architecture and allows OBS to cross-publish this 486 cross-built spec to the armv7hl repos
%if 0%{?device_target_cpu:1}
%define _target_cpu %{device_target_cpu}
%else
%define _target_cpu armv7hl
%endif

# Support build info extracted from OBS builds too
%if 0%{?_obs_build_project:1}
%define _build_flavour %(echo %{_obs_build_project} | awk -F : '{if ($NF == "testing" || $NF == "release") print $NF; else if ($NF ~ /[0-9]\.[0-9]\.[0-9]/ && NF == 3) print strdevel; else if (NF == 2) print strdevel; else print strunknown}' strdevel=devel strunknown=unknown)
%else
%define _build_flavour unknown
%endif

%define _obs_build_count %(echo %{release} | awk -F . '{if (NF >= 3) print $3; else print $1 }')
%define _obs_commit_count %(echo %{release} | awk -F . '{if (NF >= 2) print $2; else print $1 }')

%if "%{_build_flavour}" == "release"
%define _version_appendix (%{_target_cpu})
%else
%define _version_appendix (%{_target_cpu},%{_build_flavour})
%endif

# Don't run strip
%define __strip /bin/true

Summary: 	Droid HAL package for %{rpm_device}
License: 	BSD-3-Clause
Name: 		droid-hal-%{rpm_device}
Version: 	0.0.6
# timestamped releases are used only for HADK (mb2) builds
%if 0%{?_obs_build_project:1}
Release: 	1
%else
%define rel_date %(date +'%%Y%%m%%d%%H%%M')
Release: 	%{rel_date}
%endif
Provides:	droid-hal
# The repo sync service on OBS prepares a 'source tarball' of the rpm
# dir since we currently have a complex setup with subdirs which OBS
# doesn't like. This is not a problem for local builds.
Source0: 	rpm.tar.bzip2
# Ths actual droid source from the repo service when run on OBS.
# local builds don't mind if this is missing
Source40:       repo.tar.bzip2
# Reserve Source50 onwards
# Allow device specific sources to be defined using dhd_sources
%{?dhd_sources}

Group:		System
#BuildArch:	noarch
# Note that oneshot is not in mer-core (yet)
BuildRequires:  oneshot
BuildRequires:  systemd
BuildRequires:  qt5-qttools-kmap2qmap >= 5.1.0+git5
BuildRequires:  rsync
# starting from Android 8
BuildRequires:  python
%if 0%{?straggler_files:1}
Requires:       droid-hal-%{rpm_device}-detritus
%endif
# These are only required if building on OBS
%if 0%{?_obs_build_project:1}
BuildRequires:  ubu-trusty
BuildRequires:  sudo-for-abuild
%if 0%{?have_vendor_src_for_obs:1}
BuildRequires:  droid-system-vendor-obsbuild
%endif
%endif
%systemd_requires
%{_oneshot_requires_post}

%description
%{summary}.

################
%package devel
Group:	Development/Tools
# Requires: %%{name} = %%{version}-%%{release}
Provides: droid-hal-devel
Summary: Development files for droid-hal device: %{rpm_device}

%description devel
Device specific droid headers for %{rpm_device}.
Needed by libhybris

################
%package tools
Provides: droid-hal-tools
Group:	Tools
Summary: Some tools from android specific for %{device}
License: ASL 2.0
BuildRequires:  pkgconfig(zlib)

Obsoletes: droid-hal-tools <= 0.0.2

%description tools
This package contains some tools that some hardware adaptations need for
handling the android style content. This package is intended to be such
that it can be installed on the device and is not meant to be used in host/sdk
environment.

################
%package kernel
Provides: droid-hal-kernel
Group:	System
Summary: Kernel for droid-hal device: %{rpm_device}

%description kernel
Just the kernel - mainly useful if you want to make a custom img

################
%package kernel-modules
Provides: droid-hal-kernel-modules
Requires: kmod
Group:	System
Summary: Kernel modules for droid-hal device: %{rpm_device}

%description kernel-modules
Just the kernel modules

%if 0%{!?have_custom_img_boot:1}
################
%package img-boot
Provides: droid-hal-img-boot
Group:	System
Requires: oneshot
%{_oneshot_requires_post}
Summary: Boot img for droid-hal device: %{rpm_device}
%if 0%{?enable_kernel_update:1}
# The device flashing info and scripts is now in droid-configs
Requires(post): droid-config-%{rpm_device}-flashing
%endif

%description img-boot
The boot.img for device
%endif

%if 0%{!?have_custom_img_recovery:1}
################
%package img-recovery
Provides: droid-hal-img-recovery
Group:	System
Summary: Recovery image for droid-hal device: %{rpm_device}

%description img-recovery
The recovery.img for device
%endif

%if 0%{?bootloader_binary:1}
################
%package aboot
Group:  System
Summary: Bootloader for %{device} hw
%if 0%{?enable_bootloader_update:1}
# The device flashing info and scripts is now in droid-configs
Requires(post): droid-config-%{rpm_device}-flashing
%endif

%description aboot
Bootloader for %{device} hw.
%endif

%if 0%{?straggler_files:1}
################
%package detritus
Summary: Straggler files for %{device} hw
%description detritus
%{summary}.
%endif

################################################################
# Begin prep/build section

%prep
# No %%setup macro !!

%if 0%{?_obs_build_project:1}
# The OBS does not have access to 'repo' so a service does the repo init/sync
# and provides a (huge) tarball with the checked-out tree in it.
# So now drop to android_root and pretend to do a repo sync
tar xf %{SOURCE40} -C %android_root
# Clean up the repo tarball to save space
rm -f %{SOURCE40}
# Make a dummy tarball for rpm checks
mkdir dummy;(cd dummy; touch dummy; tar cvf - . | bzip2 > %{SOURCE40}); rm -rf dummy
# unpack the directories to SOURCES ... this needs to change
tar xf %{SOURCE0} -C ../SOURCES
# Clean up the rpm tarball too
rm -f %{SOURCE0}
cp %{SOURCE40} %{SOURCE0}

%if 0%{?have_vendor_src_for_obs:1}
# Copy SW binaries to the build dir (provided by droid-system-vendor-obsbuild)
cp -ar /vendor .
%endif

# In OBS the repo service leaves the rpm/* files for OBS and they just ^^
# got unpacked to ../SOURCES ... but we're used to having an rpm/ dir
# So if rpm/ is missing then we use ../SOURCES :
[ -d rpm ] || ln -s ../SOURCES rpm
%endif

%build

echo _target_cpu is %{_target_cpu}

%if 0%{!?droid_target_aarch64:1} && 0%{!?droid_target_armv7hl:1}
if (grep -q '^TARGET_ARCH := arm64' %{android_root}/device/*/*/BoardConfig*.mk); then
    echo -e \
         "\n" \
         "IMPORTANT: some devices in your Android tree are 64bit targets. If your device is aarch64,\n" \
         "           please define droid_target_aarch64 in your .spec, otherwise define droid_target_armv7hl\n" \
         "NOTE: Currently there is no Sailfish OS ARM 64bit target, so leave PORT_ARCH as armv7hl\n" \
         "      Mixed builds of 64bit Android+Linux Kernel and 32bit Sailfish OS work just fine."
    exit 1
fi
%endif

%if 0%{?_obs_build_project:1}

# Set up kernel extra version for OBS kernel builds
if EXTRAVERSION_OLD=$(cat %android_root/kernel/Makefile | grep "EXTRAVERSION ="); then
    sed -i "s/$EXTRAVERSION_OLD/EXTRAVERSION = +%{version}/g" %android_root/kernel/Makefile
elif EXTRAVERSION_OLD=$(cat %android_root/linux/kernel/Makefile | grep "EXTRAVERSION ="); then
    sed -i "s/$EXTRAVERSION_OLD/EXTRAVERSION = +%{version}/g" %android_root/linux/kernel/Makefile
else
    echo "Kernel Makefile not found, skipping EXTRAVERSION appending"
fi

# Hadk style build of android on OBS 
echo Running droid build in HABUILD_SDK
ubu-chroot -r /srv/mer/sdks/ubu "cd %android_root; %{?pre_actions}%{!?pre_actions:true}; source build/envsetup.sh; lunch %{?lunch_device}%{!?lunch_device:%{device}}%{?device_variant}; rm -f .repo/local_manifests/roomservice.xml; make %{?_smp_mflags} %{?hadk_make_target}%{!?hadk_make_target:hybris-hal}"
%endif

# Make a tmp location for built installables
rm -rf tmp
mkdir tmp

echo Verifying kernel config
# AOSP seems to use .../obj/kernel/.config not obj/KERNEL_OBJ/.config like CM : so wildcard it
hybris/mer-kernel-check/mer_verify_kernel_config \
    %{android_root}/out/target/product/%{device}/obj/*/.config


ANDROID_VERSION_MAJOR=$(awk '/PLATFORM_VERSION[A-Z0-9.]* := ([0-9.]+)/ { print $3; }' \
                          < %{android_root}/build/core/version_defaults.mk \
                          | awk -F'.' '{print $1}')

# Check if local overrides for tool mk files exist, otherwise dhd defaults are used
if test -f rpm/helpers/mkbootimg.mk; then
    export MKBOOTIMG_MK=$(pwd)/rpm/helpers/mkbootimg.mk
else
    # Starting from Android 7 mkbootimg is a python program so build only for older Android versions
    if [ $ANDROID_VERSION_MAJOR -lt "7" ]; then
        export MKBOOTIMG_MK=$(pwd)/rpm/dhd/helpers/mkbootimg.mk
    fi
fi

if test -f rpm/helpers/simg2img.mk; then
    export SIMG2IMG_MK=$(pwd)/rpm/helpers/simg2img.mk
fi

if test -f rpm/helpers/img2simg.mk; then
    export IMG2SIMG_MK=$(pwd)/rpm/helpers/img2simg.mk
fi

echo Building local tools
ANDROID_ROOT=$(readlink -e %{android_root})

if [ $ANDROID_VERSION_MAJOR -ge "8" ]; then
    $ANDROID_ROOT/build/make/tools/fs_config/fs_config_generator.py aidarray \
        $ANDROID_ROOT/system/core/libcutils/include/private/android_filesystem_config.h \
        > generated_android_filesystem_config.h
else
    echo "#include <private/android_filesystem_config.h>;" > generated_android_filesystem_config.h
fi

# files have changed in android >=8 so we need to supply different ones to the
# makefile. we also need the cstring workaround thus handling both issues here
# at once
if [ $ANDROID_VERSION_MAJOR -ge "8" ]; then
    # sparse_read.cpp doesn't include cstring and therefore fails for us
    # workaround:
    echo "#include <cstring>" > $ANDROID_ROOT/system/core/libsparse/sparse_read_fix.cpp
    echo "#include \"sparse_read.cpp\"" >> $ANDROID_ROOT/system/core/libsparse/sparse_read_fix.cpp
    IMG2SIMG_SOURCES="backed_block.c output_file.c sparse.c sparse_crc32.c sparse_err.c sparse_read_fix.cpp img2simg.c ../base/stringprintf.cpp"
    SIMG2IMG_SOURCES="backed_block.c output_file.c sparse.c sparse_crc32.c sparse_err.c sparse_read_fix.cpp simg2img.c ../base/stringprintf.cpp"
else
    IMG2SIMG_SOURCES="backed_block.c output_file.c sparse.c sparse_crc32.c sparse_err.c sparse_read.c img2simg.c"
    SIMG2IMG_SOURCES="backed_block.c output_file.c sparse.c sparse_crc32.c sparse_err.c sparse_read.c simg2img.c"
fi

pushd %{dhd_path}/helpers
make ANDROID_ROOT=$ANDROID_ROOT IMG2SIMG_SOURCES="$IMG2SIMG_SOURCES" SIMG2IMG_SOURCES="$SIMG2IMG_SOURCES"
popd

echo Building uid scripts
%{dhd_path}/helpers/usergroupgen add > %{dhd_path}/helpers/droid-user-add.sh
%{dhd_path}/helpers/usergroupgen remove > %{dhd_path}/helpers/droid-user-remove.sh

echo Building udev rules
mkdir tmp/udev.rules
# Device specific ueventd rules is the "not goldfish" one
%{dhd_path}/helpers/makeudev \
    %{android_root}/out/target/product/%{device}/root/ueventd.rc \
    $(ls %{android_root}/out/target/product/%{device}/root/ueventd.*.rc | grep -v .goldfish.rc) \
    %{android_root}/out/target/product/%{device}/vendor/ueventd.rc \
        > tmp/udev.rules/999-android-system.rules

echo Building mount units
mkdir tmp/units
# Use the makefstab and tell it what mountpoints to skip. It will
# generate .mount units which will be part of local-fs.target
# skip various mounts which are not wanted (This is just in case they creep in)
# First handle stupid spec quoting rules to get some args for makefstab
shopt -s nullglob
FSTAB_FILES="$(echo %{android_root}/out/target/product/%{device}/root/fstab.* %{android_root}/out/target/product/%{device}/root/*.rc %{android_root}/out/target/product/%{device}/vendor/etc/fstab.*)"
shopt -u nullglob


%{dhd_path}/helpers/makefstab --files $FSTAB_FILES  --skip auto none /acct /boot /cache /data /misc /recovery /staging /storage/sdcard1 /storage/usbdisk /storage/usbotg /sys/fs/cgroup /sys/fs/cgroup/memory /sys/kernel/debug  /sys/kernel/config /dev/usb-ffs/adb /tmp %{?makefstab_skip_entries} --outputdir tmp/units

echo Fixing up mount points
hybris/hybris-boot/fixup-mountpoints %{device} tmp/units/*

echo Creating hw-release
# based on http://www.freedesktop.org/software/systemd/man/os-release.html
cat > tmp/hw-release <<EOF
# This file is copied as both hw-release (analogous to os-release)
# and hw-release.vars for use at build time
MER_HA_DEVICE=%{rpm_device}
MER_HA_VENDOR=%{rpm_vendor}
MER_HA_VERSION="%{version}.%{_obs_build_count} %{_version_appendix}"
MER_HA_VERSION_ID=%{version}.%{_obs_build_count}
MER_HA_PRETTY_NAME="%{rpm_device} %{version}.%{_obs_build_count} %{_version_appendix}"
MER_HA_SAILFISH_BUILD=%{_obs_build_count}
MER_HA_SAILFISH_FLAVOUR=%{_build_flavour}
MER_HA_HOME_URL="https://sailfishos.org/"
EOF

################
%install

ANDROID_VERSION_MAJOR=$(awk '/PLATFORM_VERSION[A-Z0-9.]* := ([0-9.]+)/ { print $3; }' \
                          < %{android_root}/build/core/version_defaults.mk \
                          | awk -F'.' '{print $1}')

echo install $(cat tmp/units/all-units.txt)
rm -rf $RPM_BUILD_ROOT
# Create dir structure
mkdir -p $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/lib-dev-alog/
%if 0%{?droid_target_aarch64:1}
mkdir -p $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/lib64-dev-alog/
%endif
mkdir -p $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system
mkdir -p $RPM_BUILD_ROOT%{_libdir}/droid
mkdir -p $RPM_BUILD_ROOT%{_libdir}/droid-devel/
mkdir -p $RPM_BUILD_ROOT%{_unitdir}
mkdir -p $RPM_BUILD_ROOT/lib/udev/rules.d
mkdir -p $RPM_BUILD_ROOT/etc/udev/rules.d
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/os-release.d

mkdir -p $RPM_BUILD_ROOT%{_libdir}/modules/
mkdir -p $RPM_BUILD_ROOT%{_libdir}/droid
mkdir -p $RPM_BUILD_ROOT/%{_bindir}/droid

mkdir -p $RPM_BUILD_ROOT/img
mkdir -p $RPM_BUILD_ROOT/boot
mkdir -p $RPM_BUILD_ROOT/lib/modules

# Install tools
install -m 755 -D %{android_root}/system/core/mkbootimg/mkbootimg %{buildroot}/%{_bindir}/
install -m 755 -D %{android_root}/system/core/libsparse/simg2img %{buildroot}/%{_bindir}/
install -m 755 -D %{android_root}/system/core/libsparse/img2simg %{buildroot}/%{_bindir}/

# Install
%if 0%{?installable_zip:1}
cp -a %{android_root}/out/target/product/%{device}/system/bin/updater $RPM_BUILD_ROOT/boot/update-binary
cp -a %{android_root}/out/target/product/%{device}/hybris-updater-script $RPM_BUILD_ROOT/boot
cp -a %{android_root}/out/target/product/%{device}/hybris-updater-unpack.sh $RPM_BUILD_ROOT/boot
%endif

rsync -av %{android_root}/out/target/product/%{device}/root/. $RPM_BUILD_ROOT --exclude etc --exclude 'init.zygote*'

# We'll use the mer provided modprobe so clean that out of the way
rm -f $RPM_BUILD_ROOT/sbin/modprobe

# Now remove the mount commands from any .rc files as they're included in .mount unit files now
sed -i -e '/^[[:space:]]*mount[[:space:]]/s/^/# Removed during droid-hal-device build : /' $RPM_BUILD_ROOT/*rc
# remove the creation of /tmp to prevent wrong permissons 
sed -i -e '/^[[:space:]]*mkdir[[:space:]]\/tmp[[:space:]]*/s/^/# Removed during droid-hal-device build : /' $RPM_BUILD_ROOT/*rc

if [ $ANDROID_VERSION_MAJOR -ge "7" ]; then
mkdir -p $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/etc/init
cp -a %{android_root}/out/target/product/%{device}/system/etc/init/servicemanager.rc $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/etc/init/
echo "%{_libexecdir}/droid-hybris/system/etc/init/servicemanager.rc" > tmp/droid-hal.files
else
echo "" > tmp/droid-hal.files
fi
cp -a %{android_root}/out/target/product/%{device}/system/{bin,lib} $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/.
%if 0%{?droid_target_aarch64:1}
cp -a %{android_root}/out/target/product/%{device}/system/lib64 $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/.
%endif
# Remove kernel modules if installed under /system/lib/modules
rm -rf $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/lib/modules
cp -a %{android_root}/out/target/product/%{device}/obj/{lib,include} $RPM_BUILD_ROOT%{_libdir}/droid-devel/
rm -rf $RPM_BUILD_ROOT%{_libdir}/droid-devel/lib/*.so.toc
cp -a %{android_root}/out/target/product/%{device}/symbols $RPM_BUILD_ROOT%{_libdir}/droid-devel/

HDRS=$RPM_BUILD_ROOT%{_libdir}/droid-devel/droid-headers
mkdir -p $HDRS

# Run extract-headers.sh
%if 0%{!?provides_own_headers:1}
echo "Extracting headers for hybris"
# This is copied from libhybris and should be kept in-sync:
%{dhd_path}/helpers/extract-headers.sh -p %{_libdir}/droid-devel/droid-headers . $HDRS/ > /dev/null
if test -d "%{android_root}/rpm/header_patches"; then
    %{dhd_path}/helpers/patch-headers.sh "$HDRS" "%{android_root}/rpm/header_patches"/*.patch
fi
%else
pushd %{android_root}/rpm/droid-headers 1>/dev/null
make install DESTDIR=$RPM_BUILD_ROOT PREFIX=%{_prefix} INCLUDEDIR=%{_libdir}/droid-devel/droid-headers
popd 1>/dev/null
%endif

# Add our config into the default android-config.h
echo Making new $HDRS/android-config.h
sed '/CONFIG GOES HERE/,$d' $HDRS/android-config.h > $HDRS/android-config.h.new
cat <<EOF >> $HDRS/android-config.h.new
/* Added by Droid HAL packaging for %{rpm_device} */
%{?android_config}
EOF
# Detect QCOM_BSP and add the define into android-config.h if needed
if (grep -q '^TARGET_USES_QCOM_BSP := true' %{android_root}/device/%{vendor}/*/BoardConfig*.mk || \
   ([ $ANDROID_VERSION_MAJOR -ge "5" ] && \
     grep -q '^BOARD_USES_QCOM_HARDWARE := true' %{android_root}/device/%{vendor}/*/BoardConfig*.mk)); then

    echo Found QCOM_BSP
    cat <<EOF >> $HDRS/android-config.h.new
/* Added by automatic QCOM_BSP detection */
#define QCOM_BSP 1
#define QTI_BSP 1
EOF
fi
sed '0,/CONFIG GOES HERE/d' $HDRS/android-config.h >> $HDRS/android-config.h.new
mv $HDRS/android-config.h.new $HDRS/android-config.h

# Move the pkgconfig .pc to the correct location
%if 0%{!?provides_own_headers:1}
mkdir -p $RPM_BUILD_ROOT%{_libdir}/pkgconfig/
mv $HDRS/android-headers.pc $RPM_BUILD_ROOT%{_libdir}/pkgconfig/
%endif

# If this ever becomes unmanageable then
# grep -l dev/alog %%{android_root}/out/target/product/%{device}/system/lib/*
# libdsyscalls.so and libc.so are blacklisted
ln -s ../system/lib/{liblog.so,libcutils.so} $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/lib-dev-alog/.
%if 0%{?droid_target_aarch64:1}
ln -s ../system/lib64/{liblog.so,libcutils.so} $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/lib64-dev-alog/.
%endif

cp -a tmp/units/*.mount $RPM_BUILD_ROOT/%{_unitdir}

# Install the udev rules and supporting script
cp -a tmp/udev.rules/* $RPM_BUILD_ROOT/lib/udev/rules.d/
ln -s /dev/null $RPM_BUILD_ROOT/etc/udev/rules.d/60-persistent-v4l.rules 

# Install init-debug which will provide usb access and non-blocking telnet
INIT_SCRIPT=%{android_root}/hybris/hybris-boot/init-script

if test -f $INIT_SCRIPT; then
cp -a $INIT_SCRIPT $RPM_BUILD_ROOT/init-debug
else
cp -a %{android_root}/hybris/hybris-boot/hb/init-script $RPM_BUILD_ROOT/init-debug
fi

# droid user support This may be better done by passing a list of
# users/groups and running 'ensure_usergroups_exist newlist oldlist'
# which would preserve oldlist in %%post and delete any users no longer
# needed (unlikely!). This avoids the transient removal of uids and
# group issues
install -D %{dhd_path}/helpers/droid-user-add.sh $RPM_BUILD_ROOT%{_libdir}/droid/droid-user-add.sh
install -D %{dhd_path}/helpers/droid-user-remove.sh $RPM_BUILD_ROOT%{_libdir}/droid/droid-user-remove.sh

# Remove cruft
rm -f $RPM_BUILD_ROOT/fstab.*
rm -f $RPM_BUILD_ROOT/*goldfish*
rm -rf $RPM_BUILD_ROOT/{proc,sys,dev,sepolicy} $RPM_BUILD_ROOT/{file,seapp}_contexts
if [ $ANDROID_VERSION_MAJOR -lt "7" ]; then
rm -f $RPM_BUILD_ROOT/property_contexts
fi
rm -rf $RPM_BUILD_ROOT/{charger,res,data}

# Name this so droid-system-packager's droid-hal-startup.sh can find it
mkdir -p $RPM_BUILD_ROOT/sbin
mv $RPM_BUILD_ROOT/init $RPM_BUILD_ROOT/sbin/droid-hal-init
# Rename any symlinks to droid's /init 
find $RPM_BUILD_ROOT/sbin/ -lname ../init -execdir rm {} \; -execdir ln -s ./droid-hal-init {} \;
#mv $RPM_BUILD_ROOT/charger $RPM_BUILD_ROOT/sbin/droid-hal-charger

# for use in the -devel package
cp tmp/hw-release %{buildroot}/%{_libdir}/droid-devel/hw-release.vars

# This ghost file must exist in the installroot
touch $RPM_BUILD_ROOT/%{_libdir}/droid/droid-user-remove.sh.installed

echo "#!/bin/sh" > $RPM_BUILD_ROOT/%{_libdir}/droid/generated-post-scripts.sh
if [ $ANDROID_VERSION_MAJOR -ge "5" ] && \
   grep -q '^CONFIG_ANDROID_PARANOID_NETWORK=y' %{android_root}/out/target/product/%{device}/obj/*/.config; then

    cat >> $RPM_BUILD_ROOT/%{_libdir}/droid/generated-post-scripts.sh <<EOF
/usr/bin/groupadd-user inet || :
EOF

fi

# Kernel and module installation; to
# /boot and modules to /lib as normal
KERNEL_RELEASE=$(cat out/target/product/%{device}/*/*/include/config/kernel.release)
cp out/target/product/%{device}/kernel $RPM_BUILD_ROOT/boot/kernel-$KERNEL_RELEASE
cp out/target/product/%{device}/obj/ROOT/hybris-boot_intermediates/boot-initramfs.gz $RPM_BUILD_ROOT/boot/

echo "/boot/kernel-$KERNEL_RELEASE" > kernel.files
echo "/boot/boot-initramfs.gz" >> kernel.files

if cp out/target/product/%{device}/dt.img $RPM_BUILD_ROOT/boot/ &> /dev/null; then
    echo /boot/dt.img >> kernel.files
fi

MOD_DIR=$RPM_BUILD_ROOT/lib/modules/$KERNEL_RELEASE
mkdir -p $MOD_DIR
cp -a out/target/product/%{device}/system/lib/modules/. $MOD_DIR/. || true
cp -a out/target/product/%{device}/obj/*/modules.builtin $MOD_DIR/. || true
cp -a out/target/product/%{device}/obj/*/modules.order $MOD_DIR/. || true

# on some systems modules are in /lib/modules for some reason, lets move them to right place
mv $RPM_BUILD_ROOT/lib/modules/* $RPM_BUILD_ROOT/lib/modules/$KERNEL_RELEASE || true

# Images are installed to /boot - they'll probably be unpacked using
# rpm2cpio mostly anyhow
%if 0%{!?have_custom_img_boot:1}
cp out/target/product/%{device}/hybris-boot.img $RPM_BUILD_ROOT/boot/
%endif

%if 0%{!?have_custom_img_recovery:1}
cp out/target/product/%{device}/hybris-recovery.img $RPM_BUILD_ROOT/boot/
%endif

%if 0%{?bootloader_binary:1}
cp out/target/product/%{device}/%{?bootloader_binary} $RPM_BUILD_ROOT/boot/
%endif

# Everything is installed; get a list of the units we installed to
# allow the systemd_post to work... and then install that:
echo "$(cd $RPM_BUILD_ROOT%{_unitdir}; echo *)" > tmp/units/all-units.txt
install -D tmp/units/all-units.txt $RPM_BUILD_ROOT%{_libdir}/droid/all-units.txt

################################################################
# Begin pre/post section

%preun
for u in $(cat %{_libdir}/droid/all-units.txt); do
%systemd_preun $u
done
# Only run this during final cleanup
if [ $1 -eq 0 ]; then
    echo purging old droid users and groups
    %{_libdir}/droid/droid-user-remove.sh.installed || :
fi

%post
for u in $(cat %{_libdir}/droid/all-units.txt); do
%systemd_post $u
done
cd %{_libdir}/droid
# Upgrade: remove users using stored file, then add new ones
if [ $1 -eq 2 ]; then
    # Remove installed users (at this point droid-user-remove.sh
    # refers to the new set of UIDs)
    echo removing old droid users and groups
    ./droid-user-remove.sh.installed || :
fi
# Now for both install/update add the users and force-store a removal file
echo creating droid users and groups
./droid-user-add.sh || :
cp -f droid-user-remove.sh droid-user-remove.sh.installed

# Now ensure default user has access to various subsystems this HA provides
# These are the default android ones:
/usr/bin/groupadd-user audio || :
/usr/bin/groupadd-user graphics || :
/usr/bin/groupadd-user system || :
/usr/bin/groupadd-user input || :
/usr/bin/groupadd-user camera || :
/usr/bin/groupadd-user media || :
/usr/bin/groupadd-user mtp || :
/usr/bin/groupadd-user bluetooth || :

./generated-post-scripts.sh || :

# To add additional groups define a HA config, one can define those as part
# of additional_post_scripts macro.
%{?additional_post_scripts}

%post kernel-modules
# This runs on the device at install or in mic chroot at img build
# in chroot the kernel version is not known.
for ver in $(cd /lib/modules; echo *); do
  /sbin/depmod $ver || :
done

%if 0%{!?have_custom_img_boot:1}
%if 0%{?enable_kernel_update:1}
%post img-boot
# This oneshot is enabled only during package upgrades, as initial
# installation is done when we flash device.
if [ $1 -ne 1 ] ; then
  add-preinit-oneshot /var/lib/platform-updates/flash-bootimg.sh
fi
%endif
%endif

%if 0%{?bootloader_binary:1}
%if 0%{?enable_bootloader_update:1}
%post aboot
# This oneshot is enabled only during package upgrades, as initial
# installation is done when we flash device.
if [ $1 -ne 1 ] ; then
  add-preinit-oneshot /var/lib/platform-updates/flash-aboot.sh
fi
%endif
%endif

################################################################
# Begin files section

%files -f tmp/droid-hal.files
%defattr(-,root,root,-)
/sbin/*
# This binary should probably move to /sbin/
%{_libdir}/droid/droid-user-add.sh
%{_libdir}/droid/droid-user-remove.sh
%{_libdir}/droid/all-units.txt
# Created in %%post
# init-debug
%attr(755,root,root) /init-debug
%ghost %attr(755, root, root) %{_libdir}/droid/droid-user-remove.sh.installed
%attr(755, root, root) %{_libdir}/droid/generated-post-scripts.sh
# droid binaries
%{_libexecdir}/droid-hybris/system/bin/

# Non executable files
%defattr(644,root,root,-)
%{_unitdir}
# hybris and /dev/alog/ libraries
%{_libexecdir}/droid-hybris/system/lib/
# just /dev/alog/ libraries (for trying to run pure android apps)
%{_libexecdir}/droid-hybris/lib-dev-alog/.
# all of the above for 64bits
%if 0%{?droid_target_aarch64:1}
%{_libexecdir}/droid-hybris/system/lib64/
%{_libexecdir}/droid-hybris/lib64-dev-alog/.
%endif
/lib/udev/rules.d/*
# Droid config droppings
/*.rc
/default.prop
# Disabling v4l.rules
%{_sysconfdir}/udev/rules.d/60-persistent-v4l.rules

%files devel
%defattr(644,root,root,-)
%{_libdir}/droid-devel/
%{_libdir}/pkgconfig/*pc

%files tools
%defattr(-,root,root,-)
%{_bindir}/mkbootimg
%{_bindir}/img2simg
%{_bindir}/simg2img

%files kernel -f kernel.files
%defattr(644,root,root,-)

%files kernel-modules
%defattr(644,root,root,-)
/lib/modules

%if 0%{!?have_custom_img_boot:1}
%files img-boot
%defattr(644,root,root,-)
/boot/hybris-boot.img
%if 0%{?installable_zip:1}
/boot/update-binary
/boot/hybris-updater-script
/boot/hybris-updater-unpack.sh
%endif
%endif

%if 0%{!?have_custom_img_recovery:1}
%files img-recovery
%defattr(644,root,root,-)
/boot/hybris-recovery.img
%endif

%if 0%{?bootloader_binary:1}
%files aboot
%defattr(644,root,root,-)
/boot/%{?bootloader_binary}
%endif

%if 0%{?straggler_files:1}
%files detritus
%defattr(644,root,root,-)
%{?straggler_files}
%endif
