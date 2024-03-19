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
# dhd_path:          path to droid-hal-device git submodule
#                      defaults to rpm/dhd
# droid_target_aarch64:
#                    indicates that your device is 64bit ARM and has such
#                      Android base
# droid_target_armv7hl:
#                    indicates that your device is 32bit ARM. Only to be used
#                    with Android 6+ trees, where 64bit architecture is also
#                      available
#
# droid_src_build:
#                   indicates that this droid-hal package will be built using a
#                   droid-src source package instead of a manifest based build.
# Device capabilities:
# installable_zip:   adds zip updater scripts (default for all SFE devices)
# enable_kernel_update:
#                    ability to flash kernel partition, inspect the usage of
#                      this macro
# enable_bootloader_update:
#                    ability to flash aboot partition, inspect the usage of this
#                      macro
# enable_dtbo_update:
#                    ability to flash dtbo partition, inspect the usage of this
#                      macro
#
# enable_vendor_boot_update:
#                    ability to flash vendor_boot partition, inspect the usage of this
#                      macro
#
# Miscellaneous:
# hadk_make_target:  the target used when running make in the HABUILD_SDK on the
#                      OBS. Defaults to "hybris-hal"
# device_target_cpu: used for native builds. Normally the nested droid build env
#                      will do an old-fashioned cross-compile and produce
#                      non-x86 binaries (default armv7hl). This can be set to
#                      tell OBS what arch the binaries are. E.g. Android for
#                      Intel arch must set this (64bit ARM is autodetected below)
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
# have_custom_vendor_boot:
#                    defined when vendor_boot is packaged separately from
#                      droid-hal-device
# bootloader_binary: define bootloader binary name when aboot is packaged
#                      separately from droid-hal-device
# dtbo_binary:       override dtbo binary name. Defaults to dtbo.img
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
%if 0%{!?dhd_path:1}
%define dhd_path rpm/dhd
%endif
%if 0%{!?dtbo_binary:1}
%define dtbo_binary dtbo.img
%endif

%define __requires_exclude ^.*$
%define __provides_exclude_from ^%{_libexecdir}/droid-hybris/.*$
%define android_root .
%define dhdlibdir %{_libdir}

# Autodetect ARM 64bit architecture
%if 0%{?_obs_build_project:1}
# this always assumes "latest_", but no easy way to substr macros
%if "%{_repository}" == "latest_aarch64"
%define device_target_cpu aarch64
%endif
%else
%ifarch aarch64
%define device_target_cpu aarch64
%endif
%endif
# On the OBS this package should be built in the i486 scheduler against
# sailfish *_i486 targets.
# The prjconf should have an ExportFilter like this (sailfish has this):
#   ExportFilter: \.armv7hl\.rpm$ armv8el
# We lie about our architecture and allows OBS to cross-publish this 486 cross-built spec to the armv7hl repos
%if 0%{?device_target_cpu:1}
%if 0%{?_obs_build_project:1}
%if "%{device_target_cpu}" == "aarch64"
# Correct _libdir for OBS builds that are on i486 scheduler
%define dhdlibdir %{_prefix}/lib64
%endif
%endif
%define _target_cpu %{device_target_cpu}
%else
%define _target_cpu armv7hl
%endif

# Support build info extracted from OBS builds too
%if 0%{?_obs_build_project:1}
%define _build_flavour %(echo %{?qa_stage_name}%{!?qa_stage_name:unknown} | awk -F _ '{print $NF}')
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

# Don't terminate build if files are missing build ids
# Android builds sometimes do not generate build ids
%define _missing_build_ids_terminate_build 0

# By default built with tools supplied by the device sources
# can be change once android-tools has been updated
%bcond_without dhd_tools

Summary: 	Droid HAL package for %{rpm_device}
License: 	BSD
Name: 		droid-hal-%{rpm_device}
Version: 	0.0.6
# timestamped releases are used only for HADK (mb2) builds
%if 0%{?_obs_build_project:1}
Release: 	1
%if 0%{?droid_src_build:1}
Source0: droid-hal-%{rpm_device}-%{version}.tar.bz2
BuildRequires: droid-bin-src-full
%else
# The repo sync service on OBS prepares a 'source tarball' of the rpm
# dir since we currently have a complex setup with subdirs which OBS
# doesn't like. This is not a problem for local builds.
Source0: 	rpm.tar.bzip2
# This is actual droid source from the repo service when run on OBS.
# local builds don't mind if this is missing
Source40:       repo.tar.bzip2
%endif
%else
%define rel_date %(date +'%%Y%%m%%d%%H%%M')
Release: 	%{rel_date}
%endif
# Reserve Source50 onwards
# Allow device specific sources to be defined using dhd_sources
%{?dhd_sources}
Provides:	droid-hal

BuildRequires:  oneshot
BuildRequires:  pkgconfig(systemd)
BuildRequires:  qt5-qttools-kmap2qmap >= 5.1.0+git5
BuildRequires:  rsync
# starting from Android 8
BuildRequires:  python
%{?custom_build_requires}
Requires(pre):  droid-hal-%{rpm_device}-users
Requires:       droid-hal-%{rpm_device}-users
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
# Requires: %%{name} = %%{version}-%%{release}
Provides: droid-hal-devel
Summary: Development files for droid-hal device: %{rpm_device}

%description devel
Device specific droid headers for %{rpm_device}.
Needed by libhybris

################
%package users
Requires(pre): coreutils
Requires(post): coreutils
Requires(post): glibc-common
Requires(post): shadow-utils
Provides: hardware-adaptation-setup
Summary: Create users and groups for device: %{rpm_device}

%description users
This package provides android users and groups which are
required by droid-hal/hybris based adaptations.

################
%if %{with dhd_tools}
%package tools
Provides: droid-hal-tools
Summary: Some tools from android specific for %{device}
License: ASL 2.0
BuildRequires:  pkgconfig(zlib)
Provides: android-tools-mkbootimg
Obsoletes: droid-hal-tools <= 0.0.2

%description tools
This package contains some tools that some hardware adaptations need for
handling the android style content. This package is intended to be such
that it can be installed on the device and is not meant to be used in host/sdk
environment.
%endif

################
%package kernel
Provides: droid-hal-kernel
Summary: Kernel for droid-hal device: %{rpm_device}

%description kernel
Just the kernel - mainly useful if you want to make a custom img

################
%package kernel-modules
Provides: droid-hal-kernel-modules
Requires: kmod
Summary: Kernel modules for droid-hal device: %{rpm_device}

%description kernel-modules
Just the kernel modules

%if 0%{!?have_custom_img_boot:1}
################
%package img-boot
Provides: droid-hal-img-boot
Summary: Boot img for droid-hal device: %{rpm_device}
%if 0%{?enable_kernel_update:1}
# The device flashing info and scripts are in droid-configs
Requires: oneshot
Requires(post): droid-config-flashing
%{_oneshot_requires_post}
%endif

%description img-boot
The boot.img for device
%endif

%if 0%{!?have_custom_img_recovery:1}
################
%package img-recovery
Provides: droid-hal-img-recovery
Summary: Recovery image for droid-hal device: %{rpm_device}

%description img-recovery
The recovery.img for device
%endif

%if 0%{?bootloader_binary:1}
################
%package aboot
Summary: Bootloader for %{device} hw
%if 0%{?enable_bootloader_update:1}
# The device flashing info and scripts are in droid-configs
Requires: oneshot
Requires(post): droid-config-flashing
%{_oneshot_requires_post}
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

%package kernel-dtbo
Summary: Placeholder for %{device} flashable dtbo if applicable
%if 0%{?enable_dtbo_update:1}
# The device flashing info and scripts are in droid-configs
Requires: oneshot
Requires(post): droid-config-flashing
%{_oneshot_requires_post}
%endif

%description kernel-dtbo
%{summary}. Will stay empty, if dtbo image is not needed.

%if 0%{!?have_custom_vendor_boot:1}
%if 0%{?enable_vendor_boot_update:1}
%package vendor_boot
Summary: Placeholder for %{device} flashable vendor_boot if applicable
# The device flashing info and scripts are in droid-configs
Requires: oneshot
Requires(post): droid-config-flashing
%{_oneshot_requires_post}

%description vendor_boot
%{summary}. Will stay empty, if vendor_boot image is not needed.
%endif
%else

%package kernel-modules-vendor_boot
Summary: vendor_boot kernel modules for droid-hal device: %{rpm_device}

%description kernel-modules-vendor_boot
Just the vendor_boot kernel modules

%endif

################################################################
# Begin prep/build section

%prep
# No %%setup macro !!

%if 0%{?_obs_build_project:1}
%if 0%{?droid_src_build:1}
sudo chown -R abuild:abuild /home/abuild/src/droid/
mv /home/abuild/src/droid/* .
rm -rf rpm
tar -jxf %{SOURCE0}
cp -r droid-hal*/* .
rm -rf droid-hal*
%else
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

%{?custom_prep_cmds}
%endif

%build

echo _target_cpu is %{_target_cpu}

%if 0%{!?droid_target_aarch64:1} && 0%{!?droid_target_armv7hl:1}
if (grep -q '^TARGET_ARCH := arm64' %{android_root}/device/*/*/BoardConfig*.mk); then
    echo -e \
         "\n" \
         "IMPORTANT: some devices in your Android tree are 64bit targets. If your device is aarch64,\n" \
         "           please define droid_target_aarch64 in your .spec, otherwise define droid_target_armv7hl\n"
    exit 1
fi
%endif

%if 0%{?_obs_build_project:1}

# Set up kernel extra version for OBS kernel builds
if extraversion_old=$(grep "EXTRAVERSION =" %android_root/kernel/Makefile); then
    sed -i "s/$extraversion_old/EXTRAVERSION = +%{version}/g" %android_root/kernel/Makefile
elif extraversion_old=$(grep "EXTRAVERSION =" %android_root/linux/kernel/Makefile); then
    sed -i "s/$extraversion_old/EXTRAVERSION = +%{version}/g" %android_root/linux/kernel/Makefile
else
    echo "Kernel Makefile not found, skipping EXTRAVERSION appending"
fi

# Hadk style build of android on OBS

# We can check if we have new or old ubu-chroot by checking if it has the -V  option
# added with this version.
if ubu-chroot -V ; then
   bash="bash -c"
fi

echo Running droid build in HABUILD_SDK
ubu-chroot -r /srv/mer/sdks/ubu ${bash} "set -o errexit; cd %android_root; %{?pre_actions}%{!?pre_actions:true}; source build/envsetup.sh; lunch %{?lunch_device}%{!?lunch_device:%{device}}%{?device_variant}; rm -f .repo/local_manifests/roomservice.xml; make %{?_smp_mflags} %{?hadk_make_target}%{!?hadk_make_target:hybris-hal}"
%endif

# Make a tmp location for built installables
rm -rf tmp
mkdir tmp

echo Verifying kernel config
# AOSP seems to use .../obj/kernel/.config not obj/KERNEL_OBJ/.config like CM : so wildcard it
hybris/mer-kernel-check/mer_verify_kernel_config \
    %{android_root}/out/target/product/%{device}/obj/*/.config


if [ -f %{android_root}/build/release/build_flags.scl ]; then
android_version_major=$(awk -F '"' '/PLATFORM_VERSION_LAST_STABLE.*([0-9.]+)\"/ { print $4; }' \
                          < %{android_root}/build/release/build_flags.scl \
                          | awk -F'.' '{print $1}')
else
android_version_major=$(awk '/PLATFORM_VERSION([A-Z0-9.]*|_LAST_STABLE) := ([0-9.]+)/ { print $3; }' \
                          < %{android_root}/build/core/version_defaults.mk \
                          | awk -F'.' '{print $1}')
fi

ANDROID_ROOT=$(readlink -e %{android_root})
echo Building local tools

if [ $android_version_major -ge "8" ]; then
    $ANDROID_ROOT/build/make/tools/fs_config/fs_config_generator.py aidarray \
        $ANDROID_ROOT/system/core/libcutils/include/private/android_filesystem_config.h \
        > generated_android_filesystem_config.h
else
    echo "#include <private/android_filesystem_config.h>" > generated_android_filesystem_config.h
    if [ ! -z "$MKBOOTIMG_MK" ]; then
        sed -i -e 's/CPPFLAGS+= -I..\/include/CPPFLAGS+= -I..\/include -std=gnu99/g' $MKBOOTIMG_MK
    fi
fi

%if %{with dhd_tools}
# Check if local overrides for tool mk files exist, otherwise dhd defaults are used
if [ -f rpm/helpers/mkbootimg.mk ]; then
    export MKBOOTIMG_MK=$(pwd)/rpm/helpers/mkbootimg.mk
else
    # Starting from Android 7 mkbootimg is a python program so build only for older Android versions
    if [ $android_version_major -lt "7" ]; then
        export MKBOOTIMG_MK=$(pwd)/rpm/dhd/helpers/mkbootimg.mk
    fi
fi

if [ -f rpm/helpers/simg2img.mk ]; then
    export SIMG2IMG_MK=$(pwd)/rpm/helpers/simg2img.mk
fi

if [ -f rpm/helpers/img2simg.mk ]; then
    export IMG2SIMG_MK=$(pwd)/rpm/helpers/img2simg.mk
fi

# files have changed in android >=8 so we need to supply different ones to the
# makefile. we also need the cstring workaround thus handling both issues here
# at once
if [ $android_version_major -ge "13" ]; then
    # sparse_read.cpp doesn't include cstring and therefore fails for us
    # workaround:
    echo "#include <cstring>" > $ANDROID_ROOT/system/core/libsparse/sparse_read_fix.cpp
    echo "#include \"sparse_read.cpp\"" >> $ANDROID_ROOT/system/core/libsparse/sparse_read_fix.cpp
    IMG2SIMG_SOURCES="backed_block.c output_file.c sparse.c sparse_crc32.c sparse_err.c sparse_read_fix.cpp img2simg.c ../../libbase/mapped_file.cpp ../../libbase/stringprintf.cpp"
    SIMG2IMG_SOURCES="backed_block.c output_file.c sparse.c sparse_crc32.c sparse_err.c sparse_read_fix.cpp simg2img.c ../../libbase/mapped_file.cpp ../../libbase/stringprintf.cpp"
elif [ $android_version_major -ge "8" ]; then
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

%endif

pushd %{dhd_path}/helpers
make \
     ANDROID_ROOT=$ANDROID_ROOT \
%if %{with dhd_tools}
     IMG2SIMG_SOURCES="$IMG2SIMG_SOURCES" \
     SIMG2IMG_SOURCES="$SIMG2IMG_SOURCES" \
%else
    usergroupgen \
%endif
    %{nil}
popd

echo Building uid scripts
%{dhd_path}/helpers/usergroupgen > %{dhd_path}/helpers/droid.ids

echo Building udev rules
mkdir tmp/udev.rules
# Device specific ueventd rules is the "not goldfish" one
%{dhd_path}/helpers/makeudev \
    $(find %{android_root}/out/target/product/%{device}/system \
           %{android_root}/out/target/product/%{device}/root \
           %{android_root}/out/target/product/%{device}/vendor \
          -name ueventd.rc -o -name ueventd.\*.rc -a ! -name \*.goldfish.rc ) \
    > tmp/udev.rules/999-android-system.rules

echo Building mount units
mkdir tmp/units
# Use the makefstab and tell it what mountpoints to skip. It will
# generate .mount units which will be part of local-fs.target
# skip various mounts which are not wanted (This is just in case they creep in)
# First handle stupid spec quoting rules to get some args for makefstab
shopt -s nullglob
fstab_files="$(echo %{android_root}/out/target/product/%{device}/root/fstab* %{android_root}/out/target/product/%{device}/system/etc/init/hw/*.rc %{android_root}/out/target/product/%{device}/root/*.rc %{android_root}/out/target/product/%{device}/vendor/etc/fstab* %{android_root}/out/target/product/%{device}/system/vendor/etc/fstab*)"
shopt -u nullglob


%{dhd_path}/helpers/makefstab --files $fstab_files  --skip auto none /acct /boot /cache /data /data/user/0 /data_mirror /data_mirror/data_ce/null /data_mirror/data_de/null /data_mirror/cur_profiles /misc /recovery /staging /storage /storage/sdcard1 /storage/usbdisk /storage/usbotg /sys/fs/bpf /sys/fs/cgroup /sys/fs/cgroup/memory /sys/fs/fuse/connections /sys/fs/pstore /sys/kernel/debug /sys/kernel/config /sys/kernel/tracing /dev/cpuctl /dev/cpuset /dev/stune /dev/usb-ffs/adb /tmp %{?makefstab_skip_entries} --outputdir tmp/units

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

%{?custom_build_cmds}

################
%install

if [ -f %{android_root}/build/release/build_flags.scl ]; then
android_version_major=$(awk -F '"' '/PLATFORM_VERSION_LAST_STABLE.*([0-9.]+)\"/ { print $4; }' \
                          < %{android_root}/build/release/build_flags.scl \
                          | awk -F'.' '{print $1}')
else
android_version_major=$(awk '/PLATFORM_VERSION([A-Z0-9.]*|_LAST_STABLE) := ([0-9.]+)/ { print $3; }' \
                          < %{android_root}/build/core/version_defaults.mk \
                          | awk -F'.' '{print $1}')
fi

echo install $(cat tmp/units/all-units.txt)
# Create dir structure
mkdir -p $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/lib-dev-alog/
%if 0%{?droid_target_aarch64:1}
mkdir -p $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/lib64-dev-alog/
%endif
mkdir -p $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system
mkdir -p $RPM_BUILD_ROOT%{_includedir}/droid-devel/
mkdir -p $RPM_BUILD_ROOT%{_unitdir}
mkdir -p $RPM_BUILD_ROOT/lib/udev/rules.d
mkdir -p $RPM_BUILD_ROOT/etc/udev/rules.d
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/os-release.d

mkdir -p $RPM_BUILD_ROOT/%{_prefix}/lib/droid
mkdir -p $RPM_BUILD_ROOT/%{_bindir}/droid

mkdir -p $RPM_BUILD_ROOT/img
mkdir -p $RPM_BUILD_ROOT/boot
mkdir -p $RPM_BUILD_ROOT/lib/modules

# Install tools
if [ $android_version_major -ge "11" ]; then
    mkbootimg_dir=tools
else
    mkbootimg_dir=core
fi
if [ -e %{android_root}/system/$mkbootimg_dir/mkbootimg/mkbootimg.py ]; then
    _mkbootimg=mkbootimg.py
fi
install -m 755 \
    -D %{android_root}/system/$mkbootimg_dir/mkbootimg/${_mkbootimg:-mkbootimg} \
    %{buildroot}/%{_bindir}/mkbootimg
if [ $android_version_major -ge "13" ]; then
install -m 755 \
    -D %{android_root}/system/$mkbootimg_dir/mkbootimg/gki/generate_gki_certificate.py \
    %{buildroot}/%{_bindir}/gki/generate_gki_certificate.py
echo "%{_bindir}/gki/generate_gki_certificate.py" > tools.files
else
echo "" > tools.files
fi

install -m 755 -D %{android_root}/system/core/libsparse/simg2img %{buildroot}/%{_bindir}/
install -m 755 -D %{android_root}/system/core/libsparse/img2simg %{buildroot}/%{_bindir}/

# Install
%if 0%{?installable_zip:1}
cp -a %{android_root}/out/target/product/%{device}/system/bin/updater $RPM_BUILD_ROOT/boot/update-binary
cp -a %{android_root}/out/target/product/%{device}/hybris-updater-script $RPM_BUILD_ROOT/boot
cp -a %{android_root}/out/target/product/%{device}/hybris-updater-unpack.sh $RPM_BUILD_ROOT/boot
%endif

rsync -av %{android_root}/out/target/product/%{device}/root/. $RPM_BUILD_ROOT --exclude bin --exclude etc --exclude 'init.zygote*'

# We'll use the mer provided modprobe so clean that out of the way
rm -f $RPM_BUILD_ROOT/sbin/modprobe

if [ $android_version_major -ge "10" ]; then
    # just in case device is still using BOARD_BUILD_SYSTEM_ROOT_IMAGE, remove init:
    rm -f $RPM_BUILD_ROOT/init
    cp -a %{android_root}/out/target/product/%{device}/system/bin/init \
        $RPM_BUILD_ROOT/
fi

# Now remove the mount commands from any .rc files as they're included in .mount unit files now
sed -i -e '/^[[:space:]]*mount[[:space:]]/s/^/# Removed during droid-hal-device build : /' $RPM_BUILD_ROOT/*rc
# remove the creation of /tmp to prevent wrong permissons 
sed -i -e '/^[[:space:]]*mkdir[[:space:]]\/tmp[[:space:]]*/s/^/# Removed during droid-hal-device build : /' $RPM_BUILD_ROOT/*rc

if [ $android_version_major -ge "7" ]; then
mkdir -p $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/etc/init
cp -a %{android_root}/out/target/product/%{device}/system/etc/init/servicemanager.rc $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/etc/init/
echo "%{_libexecdir}/droid-hybris/system/etc/init/*.rc" > tmp/droid-hal.files

if [ $android_version_major -ge "11" ]; then
mkdir -p $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/etc/init/hw
cp -a %{android_root}/out/target/product/%{device}/system/etc/init/hw/init.rc $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/etc/init/hw/
cp -a %{android_root}/out/target/product/%{device}/system/etc/init/hw/init.zygote*.rc $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/etc/init/hw/

# Now remove the mount commands from any .rc files as they're included in .mount unit files now
sed -i -e '/^[[:space:]]*mount[[:space:]]/s/^/# Removed during droid-hal-device build : /' $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/etc/init/hw/*rc || true

echo "%{_libexecdir}/droid-hybris/system/etc/init/hw/*.rc" >> tmp/droid-hal.files
fi

if [ $android_version_major -ge "12" ]; then
cp -a %{android_root}/out/target/product/%{device}/system/etc/init/apexd.rc $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/etc/init/
echo "%{_libexecdir}/droid-hybris/system/etc/init/apexd.rc" >> tmp/droid-hal.files
fi

if [ -f %{android_root}/out/target/product/%{device}/system/etc/init/hybris_extras.rc ]; then
cp -a %{android_root}/out/target/product/%{device}/system/etc/init/hybris_extras.rc $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/etc/init/
fi

else
echo "" > tmp/droid-hal.files
fi

if [ $android_version_major -lt "13" ]; then
echo "/default.prop" >> tmp/droid-hal.files
fi

cp -a %{android_root}/out/target/product/%{device}/system/{bin,lib} $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/.
%if 0%{?droid_target_aarch64:1}
cp -a %{android_root}/out/target/product/%{device}/system/lib64 $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/.
%endif
if [ $android_version_major -ge "10" ]; then
    # Create empty /apex, /system/bin/apexd will then mount namespaces therein
    mkdir -p $RPM_BUILD_ROOT/apex
    echo "%dir /apex" >> tmp/droid-hal.files
    if [ $android_version_major -ge "14" ]; then
        # Create empty /bootstrap-apex
        mkdir -p $RPM_BUILD_ROOT/bootstrap-apex
        echo "%dir /bootstrap-apex" >> tmp/droid-hal.files
    fi
    # To avoid building apex (which in turn builds art),
    # we could take lib{dl,c,m}.so from:
    # out/soong/.intermediates/bionic/lib*/lib*/android_arm<COMPILER>_core_shared/lib*.so
    # but that is not elegant.
    # So we have to build apex (make com.android.runtime.release) and then:
    art_path=%{android_root}/out/soong/.intermediates/art/build
    apex_path=apex/com.android.runtime.release/android_common_com.android.runtime.release/image.apex
    if [ ! -f "$art_path/$apex_path/lib/bionic/libc.so" ]; then
        art_path=%{android_root}/out/target/product/%{device}/system
        apex_path=apex/com.android.runtime.release
    fi
    if [ ! -f "$art_path/$apex_path/lib/bionic/libc.so" ]; then
        art_path=%{android_root}/out/target/product/%{device}
        apex_path=apex/com.android.runtime
    fi

    rm -f $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/lib/lib{dl,dl_android,c,m}.so
    cp -a \
        $art_path/$apex_path/lib/bionic/lib{dl,dl_android,c,m}.so \
        $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/lib
%if 0%{?droid_target_aarch64:1}
    rm -f $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/lib64/lib{dl,dl_android,c,m}.so
    cp -a \
        $art_path/$apex_path/lib64/bionic/lib{dl,dl_android,c,m}.so \
        $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/lib64
%endif
fi

# Remove kernel modules if installed under /system/lib/modules
rm -rf $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/lib/modules
cp -a %{android_root}/out/target/product/%{device}/obj/include $RPM_BUILD_ROOT%{_includedir}/droid-devel/ || true
rm -rf $RPM_BUILD_ROOT%{_includedir}/droid-devel/lib/*.so.toc

# Omit audioflingerglue bits, they get packaged via audioflingerglue-localbuild
rm -f $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/{lib,lib64}/libaudioflingerglue.so
rm -f $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/bin/miniafservice
# Omit droidmedia bits, they get packaged via droidmedia-localbuild
rm -f $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/{lib,lib64}/libdroidmedia.so
rm -f $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/{lib,lib64}/libminisf.so
rm -f $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/bin/minimediaservice
rm -f $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/bin/minisfservice

hdrs=$RPM_BUILD_ROOT%{_includedir}/droid-devel/droid-headers
mkdir -p $hdrs

# Run extract-headers.sh
%if 0%{!?provides_own_headers:1}
echo "Extracting headers for hybris"
# This is copied from libhybris and should be kept in-sync:
%{dhd_path}/helpers/extract-headers.sh -p %{_includedir}/droid-devel/droid-headers . $hdrs/ > /dev/null
if [ -d "%{android_root}/rpm/header_patches" ]; then
    %{dhd_path}/helpers/patch-headers.sh "$hdrs" "%{android_root}/rpm/header_patches"/*.patch
fi
%else
pushd %{android_root}/rpm/droid-headers 1>/dev/null
make install DESTDIR=$RPM_BUILD_ROOT PREFIX=%{_prefix} INCLUDEDIR=%{_includedir}/droid-devel/droid-headers
popd 1>/dev/null
%endif

# Add our config into the default android-config.h
echo Making new $hdrs/android-config.h
token_line=$(sed -n '/CONFIG GOES HERE/=' $hdrs/android-config.h)
sed "$token_line,\$ d" $hdrs/android-config.h > $hdrs/android-config.h.new
cat <<EOF >> $hdrs/android-config.h.new
/* Added by Droid HAL packaging for %{rpm_device} */
%{?android_config}
EOF
# Detect QCOM_BSP and add the define into android-config.h if needed
if (grep -q '^TARGET_USES_QCOM_BSP := true' %{android_root}/device/%{vendor}/*/BoardConfig*.mk || \
   ([ $android_version_major -ge "5" ] && \
     grep -q '^BOARD_USES_QCOM_HARDWARE := true' %{android_root}/device/%{vendor}/*/BoardConfig*.mk)); then

    echo Found QCOM_BSP
    cat <<EOF >> $hdrs/android-config.h.new
/* Added by automatic QCOM_BSP detection */
#define QCOM_BSP 1
#define QTI_BSP 1
EOF
fi
if [ $android_version_major -ge "13" ]; then
    cat <<EOF >> $hdrs/android-config.h.new
/* For Android 13+ */
#define __BIONIC_VERSIONER
#define _Nonnull
#define _Nullable

#include <android/versioning.h>
EOF
fi
sed "1,$token_line d" $hdrs/android-config.h >> $hdrs/android-config.h.new
mv $hdrs/android-config.h.new $hdrs/android-config.h

# Move the pkgconfig .pc to the correct location
%if 0%{!?provides_own_headers:1}
mkdir -p $RPM_BUILD_ROOT%{dhdlibdir}/pkgconfig/
%if 0%{?_obs_build_project:1}
# this hack fixes OBS i586 worker failure
mkdir -p $RPM_BUILD_ROOT%{_prefix}/lib/pkgconfig/ || true
cp $hdrs/android-headers.pc $RPM_BUILD_ROOT%{_prefix}/lib/pkgconfig/
%endif
mv $hdrs/android-headers.pc $RPM_BUILD_ROOT%{dhdlibdir}/pkgconfig/
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
init_script=%{android_root}/hybris/hybris-boot/init-script

if [ -f $init_script ] ; then
cp -a $init_script $RPM_BUILD_ROOT/init-debug
else
cp -a %{android_root}/hybris/hybris-boot/hb/init-script $RPM_BUILD_ROOT/init-debug
fi

# droid user support
install -D %{dhd_path}/helpers/droid.ids $RPM_BUILD_ROOT/%{_prefix}/lib/droid/droid.ids

# Remove cruft, unless the file is listed in straggler_files

_remove_cruft() {
%if 0%{?straggler_files:1}
    # Cheat a bit and use substring search for lookup
    local straggler_lookup=
    for straggler_file in $(echo "%{?straggler_files}" | xargs -n1); do
        straggler_lookup="${straggler_lookup}<${straggler_file}>"
    done

    while [ $# -gt 0 ]; do
        for _f in $(ls -1 -d ${RPM_BUILD_ROOT}${1} 2>/dev/null); do
            local fn="${_f/#${RPM_BUILD_ROOT}}"
            if [[ ! "${straggler_lookup}" =~ "<${fn//+/\\+}>" ]]; then
                rm -rf ${_f}
            fi
        done
        shift
    done
%else
    while [ $# -gt 0 ]; do
        rm -rf ${RPM_BUILD_ROOT}${1}
        shift
    done
%endif
}

# Make sure to add the files in quotations so that string expansion is not done here.
# remove odm subdirs in case they exist to avoid problems when installing droid-hal (odm is ro)
_remove_cruft   "/fstab.*"          \
                "/*goldfish*"       \
                "/proc"             \
                "/sys"              \
                "/dev"              \
                "/sepolicy"         \
                "/file_contexts"    \
                "/seapp_contexts"   \
                "/charger"          \
                "/res"              \
                "/data"             \
                "/odm"              \
                "/sbin/mkdosfs"
if [ $android_version_major -lt "7" ]; then
    _remove_cruft "/property_contexts"
fi

mkdir -p $RPM_BUILD_ROOT/data

# Name this so droid-system-packager's droid-hal-startup.sh can find it
mkdir -p $RPM_BUILD_ROOT/sbin
mv $RPM_BUILD_ROOT/init $RPM_BUILD_ROOT/sbin/droid-hal-init
# Rename any symlinks to droid's /init 
for link in $(find $RPM_BUILD_ROOT/sbin/ -type l); do
    if [ "$(readlink $link)" == "../init" ]; then
        rm "$link"
        ln -s ./droid-hal-init "$link"
    fi
done
#mv $RPM_BUILD_ROOT/charger $RPM_BUILD_ROOT/sbin/droid-hal-charger

# for use in the -devel package
cp tmp/hw-release %{buildroot}/%{_includedir}/droid-devel/hw-release.vars

# Store the groups that users need to be in to access various subsystems this HA provides
# These are the default android ones:

mkdir -p %{buildroot}/%{_prefix}/lib/droid/hw-group.d
cat >> %{buildroot}/%{_prefix}/lib/droid/hw-group.d/default.list <<EOF
audio
bluetooth
camera
graphics
input
media
media_rw
mtp
EOF

# If user needs to be removed from a certain group define the groups
# here. If a user is added to a group listed in additional_ha_groups
# macro then the removal is not done, even when the group is listed
# here.

cat >> %{buildroot}/%{_prefix}/lib/droid/hw-group.d/default.list.remove <<EOF
system
EOF

if [ $android_version_major -ge "5" ] && \
   grep -q '^CONFIG_ANDROID_PARANOID_NETWORK=y' %{android_root}/out/target/product/%{device}/obj/*/.config; then
    echo "inet" > %{buildroot}/%{_prefix}/lib/droid/hw-group.d/inet.list
fi

# To add additional groups needed by device adaptation, one can define those as part
# of additional_ha_groups macro.

if [ -n "%{?additional_ha_groups}" ]; then
cat >> %{buildroot}/%{_prefix}/lib/droid/hw-group.d/device.list <<EOF
%{?additional_ha_groups}
EOF
fi

# To remove additional groups needed by device adaptation, one can define those as part
# of additional_ha_groups_remove macro.

if [ -n "%{?additional_ha_groups_remove}" ]; then
cat >> %{buildroot}/%{_prefix}/lib/droid/hw-group.d/device.list.remove <<EOF
%{?additional_ha_groups_remove}
EOF
fi

# Kernel and module installation; to
# /boot and modules to /lib as normal
kernel_release=$(sort -u out/target/product/%{device}/*/*/include/config/kernel.release)
cp out/target/product/%{device}/kernel $RPM_BUILD_ROOT/boot/kernel-$kernel_release
cp out/target/product/%{device}/obj/ROOT/hybris-boot_intermediates/boot-initramfs.gz $RPM_BUILD_ROOT/boot/

echo "/boot/kernel-$kernel_release" > kernel.files
echo "/boot/boot-initramfs.gz" >> kernel.files

if cp out/target/product/%{device}/dt.img $RPM_BUILD_ROOT/boot/ &> /dev/null; then
    echo /boot/dt.img >> kernel.files
fi

if cp out/target/product/%{device}/dtb.img $RPM_BUILD_ROOT/boot/ &> /dev/null; then
    echo /boot/dtb.img >> kernel.files
fi

touch dtbo.files

if cp out/target/product/%{device}/%{dtbo_binary} $RPM_BUILD_ROOT/boot/dtbo.img &> /dev/null; then
    echo /boot/dtbo.img > dtbo.files
fi

%if 0%{!?have_custom_vendor_boot:1}
%if 0%{?enable_vendor_boot_update:1}
cp out/target/product/%{device}/vendor_boot.img $RPM_BUILD_ROOT/boot/
%endif
%endif

touch boot.files

mod_dir=$RPM_BUILD_ROOT/lib/modules/$kernel_release
mkdir -p $mod_dir
cp -a out/target/product/%{device}/system/lib/modules/. $mod_dir/. || true
cp -a out/target/product/%{device}/system/vendor/lib/modules/. $mod_dir/. || true
cp -a out/target/product/%{device}/vendor/lib/modules/. $mod_dir/. || true
cp -a out/target/product/%{device}/vendor_dlkm/lib/modules/. $mod_dir/. || true
cp -a out/target/product/%{device}/obj/*/modules.builtin $mod_dir/. || true
cp -a out/target/product/%{device}/obj/*/modules.order $mod_dir/. || true

%if 0%{?have_custom_vendor_boot:1}
echo "" > kernel-modules-vendor_boot.files
if [ -d out/target/product/%{device}/vendor_ramdisk/lib/modules ]; then
    vendor_boot_mod_dir=$RPM_BUILD_ROOT/lib/vendor_boot_modules
    mkdir -p $vendor_boot_mod_dir
    cp -a out/target/product/%{device}/vendor_ramdisk/lib/modules/. $vendor_boot_mod_dir/. || true
    echo "/lib/vendor_boot_modules/*" > kernel-modules-vendor_boot.files
fi
%endif

# on some systems modules are in /lib/modules for some reason, lets move them to right place
mv $RPM_BUILD_ROOT/lib/modules/* $RPM_BUILD_ROOT/lib/modules/$kernel_release || true

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
install -D tmp/units/all-units.txt $RPM_BUILD_ROOT%{_prefix}/lib/droid/all-units.txt

%{?custom_install_cmds}

################################################################
# Begin pre/post section

%preun
for u in $(cat %{_prefix}/lib/droid/all-units.txt); do
%systemd_preun $u
done

%post
for u in $(cat %{_prefix}/lib/droid/all-units.txt); do
%systemd_post $u
done

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
  add-preinit-oneshot /var/lib/platform-updates/flash-bootimg.sh || :
fi
%endif
%endif

%if 0%{?bootloader_binary:1}
%if 0%{?enable_bootloader_update:1}
%post aboot
# This oneshot is enabled only during package upgrades, as initial
# installation is done when we flash device.
if [ $1 -ne 1 ] ; then
  add-preinit-oneshot /var/lib/platform-updates/flash-aboot.sh || :
fi
%endif
%endif

%if 0%{?enable_dtbo_update:1}
%post kernel-dtbo
# This oneshot is enabled only during package upgrades, as initial
# installation is done when we flash device.
if [ $1 -ne 1 ] ; then
  add-preinit-oneshot /var/lib/platform-updates/flash-dtbo.sh || :
fi
%endif

%if 0%{!?have_custom_vendor_boot:1}
%if 0%{?enable_vendor_boot_update:1}
%post vendor_boot
# This oneshot is enabled only during package upgrades, as initial
# installation is done when we flash device.
if [ $1 -ne 1 ] ; then
  add-preinit-oneshot /var/lib/platform-updates/flash-vendor_boot.sh || :
fi
%endif
%endif

%pre users
#Compatibility with old droid-hal version - convert previous format
#of droid-specific user/group information to current one.
if [ $1 -gt 1 -a \
     -f "%{_prefix}/lib/droid/droid-user-remove.sh.installed" ]; then
  echo '#!/bin/sh' > "%{_prefix}/lib/droid/droid-user-remove.sh.installed"
fi

%post users
cd %{_prefix}/lib/droid

# NOTE: We do not delete groups/users in any case since that would cause issues
# with users being removed from groups where they should really be in.

echo creating droid users and groups
[ -e droid.ids ] && cat droid.ids | while read name id; do
    if ! getent group $name; then
        if getent group $id; then
            other_name=$(getent group $id 2>/dev/null |cut -d":" -f1)
            echo "Group $name did not exist yet, but another group has the same id ($id, $other_name), renaming that group"
            groupmod -g $id -n $name $other_name || :
        else
            echo "Group $name did not exist yet"
            groupadd -g $id $name || :
        fi
    else
        echo "Group $name already existed, modifying it"
        groupmod -g $id $name || :
    fi
    if ! getent passwd $name; then
        if getent passwd $id; then
            other_name=$(getent passwd $id 2>/dev/null |cut -d":" -f1)
            echo "User $name did not exist yet, but another user has the same id ($id, $other_name), renaming that user"
            usermod -s /sbin/nologin -d / -u $id -g $id -l $name $other_name || :
        else
            echo "User $name did not exist yet"
            useradd -M -N -s /sbin/nologin -d / -u $id -g $id $name || ::
        fi
    else
        echo "User $name already existed, modifying it"
        usermod -s /sbin/nologin -d / -u $id -g $id $name || :
    fi
done

################################################################
# Begin files section

%files -f tmp/droid-hal.files
%defattr(-,root,root,-)
%dir /data
/sbin/*
# This binary should probably move to /sbin/
%{_prefix}/lib/droid/all-units.txt
# Created in %%post
# init-debug
%attr(755,root,root) /init-debug
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
# Disabling v4l.rules
%{_sysconfdir}/udev/rules.d/60-persistent-v4l.rules

%files users
%dir %{_prefix}/lib/droid
%dir %{_prefix}/lib/droid/hw-group.d
%{_prefix}/lib/droid/droid.ids
%{_prefix}/lib/droid/hw-group.d/*.list*

%files devel
%defattr(644,root,root,-)
%{_includedir}/droid-devel/
%if 0%{?_obs_build_project:1}
# this hack fixes OBS i586 worker failure
%{_prefix}/lib*/pkgconfig/*pc
%else
%{dhdlibdir}/pkgconfig/*pc
%endif

%if %{with dhd_tools}
%files tools -f tools.files
%defattr(-,root,root,-)
%{_bindir}/mkbootimg
%{_bindir}/img2simg
%{_bindir}/simg2img
%endif

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

%files kernel-dtbo -f dtbo.files
%defattr(644,root,root,-)

%if 0%{!?have_custom_vendor_boot:1}
%if 0%{?enable_vendor_boot_update:1}
%files vendor_boot
/boot/vendor_boot.img
%defattr(644,root,root,-)
%endif
%else
%files kernel-modules-vendor_boot -f kernel-modules-vendor_boot.files
%defattr(644,root,root,-)
%endif
