# This file should be %%included into a device specific spec file
# where macros are defined:
# device: should be the CM codename or the AOSP TARGET_PRODUCT
# vendor: determine the directory used for ./device/<vendor>/<device>
# device_pretty: User-visible model name of the device
# vendor_pretty: User-visible manufacturer name of the device
# hadk_make_target: the target used when running make in the HABUILD_SDK on the OBS. Defaults to "hybris-hal"
# device_target_cpu: Used for native builds. Normally the nested droid build env will do an old-fashioned cross-compile and produce non-x86 binaries (default armv7hl). This can be set to tell OBS what arch the binaries are. Eg Android for Intel arch must set this.
# device_variant: for AOSP this is used as the TARGET_BUILD_VARIANT for lunch

%define __provides_exclude_from ^%{_libexecdir}/droid-hybris/.*$
%define android_root .
%define dhd_path rpm/dhd
%define rel_date %(date +'%%Y%%m%%d%%H%%M')

# On the OBS this package should be built in the i486 scheduler against
# mer/sailfish *_i486 targets.
# The prjconf should have an ExportFilter like this (mer/sailfish has this):
#   ExportFilter: \.armv7hl\.rpm$ armv8el
# We lie about our architecture and allows OBS to cross-publish this 486 cross-built spec to the armv7hl repos
%if 0%{?device_target_cpu:1}
%define my_target_cpu %{device_target_cpu}
%else
%define _target_cpu armv7hl
%endif

# Support build info extracted from OBS builds too
%if 0%{?_obs_build_project:1}
%define _build_flavour %(echo %{_obs_build_project} | awk -F : '{if (NF == 3) print $3; else if (NF == 2) print strdevel; else print strunknown}' strdevel=devel strunknown=unknown)
%else
%define _build_flavour unknown
%endif

%define _obs_build_count %(echo %{release} | awk -F . '{if (NF >= 3) print $3; else print $1 }')
%define _obs_commit_count %(echo %{release} | awk -F . '{if (NF >= 2) print $2; else print $1 }')

%if %{_build_flavour} == release
%define _version_appendix (%{_target_cpu})
%else
%define _version_appendix (%{_target_cpu},%{_build_flavour})
%endif

%define board_mapping_dir %{_datadir}/ssu/board-mappings.d
%define board_mapping_file %{board_mapping_dir}/05-%{vendor}-%{device}.ini

# Don't run strip
%define __strip /bin/true

Summary: 	Droid HAL package for %{device}
License: 	BSD-3-Clause
Name: 		droid-hal-%{device}
Version: 	0.0.6
Release: 	%{rel_date}
Provides:	droid-hal
Provides:	flash-partition
# The repo sync service on OBS prepares a 'source tarball' of the rpm
# dir since we currently have a complex setup with subdirs which OBS
# doesn't like. This is not a problem for local builds.
Source0: 	rpm.tar.bzip2
Source10:       droid-hal-device.inc
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
BuildRequires:  repomd-pattern-builder
# These are only required if building on OBS
%if 0%{?_obs_build_project:1}
BuildRequires:  ubu-trusty
BuildRequires:  sudo-for-abuild
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
Summary: Development files for droid-hal device: %{device}

%description devel
Device specific droid headers for %{device}.
Needed by libhybris

################
%package sailfish-config
Provides: droid-hal-sailfish-config
Group:	System
BuildArch:	noarch
Requires: %{name} = %{version}-%{release}
Requires: oneshot
Requires: dconf
Summary: Per device configuration for sailfish for droid-hal device: %{device}

%description sailfish-config
Configure sailfish eg naturally landscape devices like mako

################
%package kernel
Provides: droid-hal-kernel
Group:	System
Summary: Kernel for droid-hal device: %{device}

%description kernel
Just the kernel - mainly useful if you want to make a custom img

################
%package kernel-modules
Provides: droid-hal-kernel-modules
Requires: kmod
Group:	System
Summary: Kernel modules for droid-hal device: %{device}

%description kernel-modules
Just the kernel modules

################
%package img-boot
Provides: droid-hal-img-boot
Group:	System
Requires: flash-partition
Requires: oneshot
%{_oneshot_requires_post}
Summary: Boot img for droid-hal device: %{device}

%description img-boot
The boot.img for device

################
%package img-recovery
Provides: droid-hal-img-recovery
Group:	System
BuildArch:	noarch
Summary: Recovery image for droid-hal device: %{device}

%description img-recovery
The recovery.img for device

################
%package pulseaudio-settings
Summary:    PulseAudio settings for %{device} hw
Requires:   pulseaudio >= 4.0
Requires:   pulseaudio-modules-nemo-parameters >= 4.0.11
Requires:   pulseaudio-modules-nemo-stream-restore >= 4.0.11
Requires:   pulseaudio-modules-nemo-mainvolume >= 4.0.11
Requires:   pulseaudio-modules-droid >= 4.0.6
Requires:   pulseaudio-policy-enforcement >= 4.0.8
Provides:   pulseaudio-settings

%description pulseaudio-settings
%{summary}.

################
%package policy-settings
Summary:    Policy settings for %{device} hw
Requires:   ohm >= 1.1.16
Requires:   ohm-plugins-misc
Requires:   ohm-plugins-dbus
Requires:   ohm-plugin-telephony
Requires:   ohm-plugin-signaling
Requires:   ohm-plugin-media
Requires:   ohm-plugin-accessories
Requires:   ohm-plugin-resolver
Requires:   ohm-plugin-ruleengine
Requires:   ohm-plugin-profile
Requires:   pulseaudio-modules-nemo-common >= 4.0.11
Requires:   pulseaudio-policy-enforcement >= 4.0.8
Requires:   policy-settings-common >= 0.2.0
Obsoletes:  ohm-config <= 1.1.15
# ohm-configs-default should not be installed ever, thus no version
# specification defined here.
Obsoletes:  ohm-configs-default
Provides:   ohm-configs > 1.1.15
Provides:   policy-settings

%description policy-settings
%{summary}.

################
%package kickstart-configuration
Summary:    Kickstart configuration for %{device}
Requires:   ssu-kickstart-configuration-jolla
Provides:   droid-hal-kickstart-configuration

%description kickstart-configuration
%{summary}.

################

%package preinit-plugin
Summary:    Preinit plugins for %{device}
Provides:   preinit-plugins

%description preinit-plugin
%{summary}.

################################################################
%package patterns
Summary:    Repository patterns for %{device} hw
Provides:   package-groups

%description patterns
%{summary}.

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

# In OBS the repo service leaves the rpm/* files for OBS and they just ^^
# got unpacked to ../SOURCES ... but we're used to having an rpm/ dir
# So if rpm/ is missing then we use ../SOURCES :
[ -d rpm ] || ln -s ../SOURCES rpm
%endif

%build

echo _target_cpu is %{_target_cpu}

%if 0%{?_obs_build_project:1}
# Hadk style build of android on OBS 
echo Running droid build in HABUILD_SDK
ubu-chroot -r /srv/mer/sdks/ubu "cd %android_root; source build/envsetup.sh; lunch %{device}%{?device_variant}; rm -f .repo/local_manifests/roomservice.xml; make %{?_smp_mflags} %{?hadk_make_target}%{!?hadk_make_target:hybris-hal}"
%endif

# Make a tmp location for built installables
rm -rf tmp
mkdir tmp

echo Verifying kernel config
# AOSP seems to use .../obj/kernel/.config not obj/KERNEL_OBJ/.config like CM : so wildcard it
hybris/mer-kernel-check/mer_verify_kernel_config \
    %{android_root}/out/target/product/%{device}/obj/*/.config

echo Building local tools
ANDROID_ROOT=$(readlink -e %{android_root})
(cd %{dhd_path}/helpers; make ANDROID_ROOT=$ANDROID_ROOT)

echo Building uid scripts
%{dhd_path}/helpers/usergroupgen add > %{dhd_path}/helpers/droid-user-add.sh
%{dhd_path}/helpers/usergroupgen remove > %{dhd_path}/helpers/droid-user-remove.sh

echo Building udev rules
mkdir tmp/udev.rules
# Device specific ueventd rules is the "not goldfish" one
%{dhd_path}/helpers/makeudev \
    %{android_root}/out/target/product/%{device}/root/ueventd.rc \
    $(ls %{android_root}/out/target/product/%{device}/root/ueventd.*.rc | grep -v .goldfish.rc) \
        > tmp/udev.rules/999-android-system.rules

echo Building mount units
mkdir tmp/units
# Use the makefstab and tell it what mountpoints to skip. It will
# generate .mount units which will be part of local-fs.target
# skip various mounts which are not wanted (This is just in case they creep in)
# First handle stupid spec quoting rules to get some args for makefstab
shopt -s nullglob
FSTAB_FILES="$(echo %{android_root}/out/target/product/%{device}/root/fstab.* %{android_root}/out/target/product/%{device}/root/*.rc)"
shopt -u nullglob


%{dhd_path}/helpers/makefstab --files $FSTAB_FILES  --skip auto /acct /boot /cache /data /misc /recovery /staging /storage/usbdisk /sys/fs/cgroup /sys/fs/cgroup/memory /sys/kernel/debug  /sys/kernel/config --outputdir tmp/units

echo Fixing up mount points
hybris/hybris-boot/fixup-mountpoints %{device} tmp/units/*

echo Creating hw-release
# based on http://www.freedesktop.org/software/systemd/man/os-release.html
cat > tmp/hw-release <<EOF
# This file is copied as both hw-release (analogous to os-release)
# and hw-release.vars for use at build time
MER_HA_DEVICE=%{device}
MER_HA_VENDOR=%{vendor}
MER_HA_VERSION="%{version}.%{_obs_build_count} %{_version_appendix}"
MER_HA_VERSION_ID=%{version}.%{_obs_build_count}
MER_HA_PRETTY_NAME="%{device} %{version}.%{_obs_build_count} %{_version_appendix}"
MER_HA_SAILFISH_BUILD=%{_obs_build_count}
MER_HA_SAILFISH_FLAVOUR=%{_build_flavour}
MER_HA_HOME_URL="https://sailfishos.org/"
EOF

# Grab a snapshot of the git state from repo
if [ -e ./repo_service_manifest.xml ] ; then
   # The OBS service helpfully dropped a repo manifest file here:
   mv ./repo_service_manifest.xml tmp/manifest.xml
else
   ./.repo/repo/repo manifest -r -o tmp/manifest.xml
fi

################
%install
echo install $(cat tmp/units/all-units.txt)
rm -rf $RPM_BUILD_ROOT
# Create dir structure
mkdir -p $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/lib-dev-alog/
mkdir -p $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system
mkdir -p $RPM_BUILD_ROOT%{_libdir}/droid
mkdir -p $RPM_BUILD_ROOT%{_libdir}/droid-devel/
mkdir -p $RPM_BUILD_ROOT%{_unitdir}
mkdir -p $RPM_BUILD_ROOT/lib/udev/rules.d
mkdir -p $RPM_BUILD_ROOT/etc/udev/rules.d
mkdir -p $RPM_BUILD_ROOT/%{_sharedstatedir}/environment/compositor
mkdir -p $RPM_BUILD_ROOT/%{_sharedstatedir}/environment/nemo
mkdir -p $RPM_BUILD_ROOT/%{_oneshotdir}
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/os-release.d

mkdir -p $RPM_BUILD_ROOT%{_libdir}/modules/
mkdir -p $RPM_BUILD_ROOT%{_libdir}/droid
mkdir -p $RPM_BUILD_ROOT/%{_bindir}/droid

mkdir -p $RPM_BUILD_ROOT/img
mkdir -p $RPM_BUILD_ROOT/boot
mkdir -p $RPM_BUILD_ROOT/lib/modules
mkdir -p $RPM_BUILD_ROOT%{_libdir}/startup/preinit

# Install
cp -a %{android_root}/out/target/product/%{device}/system/bin/updater $RPM_BUILD_ROOT/boot/update-binary
cp -a %{android_root}/out/target/product/%{device}/hybris-updater-script $RPM_BUILD_ROOT/boot
cp -a %{android_root}/out/target/product/%{device}/hybris-updater-unpack.sh $RPM_BUILD_ROOT/boot

cp -a %{android_root}/out/target/product/%{device}/root/. $RPM_BUILD_ROOT/

# Now remove the mount commands from any .rc files as they're included in .mount unit files now
sed -i -e '/^[[:space:]]*mount[[:space:]]/s/^/# Removed during droid-hal-device build : /' $RPM_BUILD_ROOT/*rc

cp -a %{android_root}/out/target/product/%{device}/system/{bin,lib} $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/.
cp -a %{android_root}/out/target/product/%{device}/obj/{lib,include} $RPM_BUILD_ROOT%{_libdir}/droid-devel/
cp -a %{android_root}/out/target/product/%{device}/symbols $RPM_BUILD_ROOT%{_libdir}/droid-devel/

HDRS=$RPM_BUILD_ROOT%{_libdir}/droid-devel/droid-headers
mkdir -p $HDRS

# Store the 'as built' manifest
cp tmp/manifest.xml $HDRS/repo-manifest.xml

# Run extract-headers.sh
echo "Extracting headers for hybris"
# This is copied from libhybris and should be kept in-sync:
%{dhd_path}/helpers/extract-headers.sh -p %{_libdir}/droid-devel/droid-headers . $HDRS/ > /dev/null

# Add our config into the default android-config.h
echo Making new $HDRS/android-config.h
sed '/CONFIG GOES HERE/,$d' $HDRS/android-config.h > $HDRS/android-config.h.new
cat <<EOF >> $HDRS/android-config.h.new
/* Added by Droid HAL packaging for %{device} */
%{?android_config}
EOF
sed '0,/CONFIG GOES HERE/d' $HDRS/android-config.h >> $HDRS/android-config.h.new
mv $HDRS/android-config.h.new $HDRS/android-config.h

# Move the pkgconfig .pc to the correct location
mkdir -p $RPM_BUILD_ROOT%{_libdir}/pkgconfig/
mv $HDRS/android-headers.pc $RPM_BUILD_ROOT%{_libdir}/pkgconfig/

# If this ever becomes unmanageable then
# grep -l dev/alog %%{android_root}/out/target/product/%{device}/system/lib/*
# libdsyscalls.so and libc.so are blacklisted
ln -s ../system/lib/{liblog.so,libcutils.so} $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/lib-dev-alog/.

cp -a tmp/units/*.mount $RPM_BUILD_ROOT/%{_unitdir}
cp -a %{dhd_path}/dsp/systemd/* $RPM_BUILD_ROOT/%{_unitdir}

# Install the udev rules and supporting script
cp -a tmp/udev.rules/* $RPM_BUILD_ROOT/lib/udev/rules.d/
ln -s /dev/null $RPM_BUILD_ROOT/etc/udev/rules.d/60-persistent-v4l.rules 

# Install init-debug which will provide usb access and non-blocking telnet
cp -a %{android_root}/hybris/hybris-boot/init-script $RPM_BUILD_ROOT/init-debug

# droid user support This may be better done by passing a list of
# users/groups and running 'ensure_usergroups_exist newlist oldlist'
# which would preserve oldlist in %%post and delete any users no longer
# needed (unlikely!). This avoids the transient removal of uids and
# group issues
install -D %{dhd_path}/helpers/droid-user-add.sh $RPM_BUILD_ROOT%{_libdir}/droid/droid-user-add.sh
install -D %{dhd_path}/helpers/droid-user-remove.sh $RPM_BUILD_ROOT%{_libdir}/droid/droid-user-remove.sh

# Add the dsp support binaries
cp %{dhd_path}/dsp/bin/* $RPM_BUILD_ROOT/%{_bindir}/droid

# Preinit plugins
install -m 0744  %{dhd_path}/dsp/preinit/* $RPM_BUILD_ROOT%{_libdir}/startup/preinit/
ln -s /bin/false $RPM_BUILD_ROOT%{_libdir}/startup/preinit/is_total_erase_supported


# Remove cruft
rm -f $RPM_BUILD_ROOT/fstab.*
rm -f $RPM_BUILD_ROOT/*goldfish*
rm -rf $RPM_BUILD_ROOT/{proc,sys,dev,sepolicy} $RPM_BUILD_ROOT/{file,property,seapp}_contexts
rm -rf $RPM_BUILD_ROOT/{charger,res,data}

# Name this so droid-system-packager's droid-hal-startup.sh can find it
mkdir -p $RPM_BUILD_ROOT/sbin
mv $RPM_BUILD_ROOT/init $RPM_BUILD_ROOT/sbin/droid-hal-init
# Rename any symlinks to droid's /init 
find $RPM_BUILD_ROOT/sbin/ -lname ../init -execdir echo rm {} \; -execdir echo "ln -s" ./droid-hal-init {} \;
#mv $RPM_BUILD_ROOT/charger $RPM_BUILD_ROOT/sbin/droid-hal-charger

# Amalgamate configs files from device-specific and all- trees
# Retain permissions:
echo "%defattr(-,root,root,-)" > tmp/conf.files
# Add files that you will handle manually (per-device and/or per-package) to this | separated var:
EXCLUDES="/var/lib/platform-updates/flash-bootimg.sh"

# Prefer files from device-$VENDOR-$DEVICE-configs in the HA specific
# area over device-configs-all in the dhd-common area
for config_dir in %{dhd_path}/device-configs-all rpm/device-%{vendor}-%{device}-configs; do
    if [ -d $config_dir ]; then
        (cd $config_dir; find . \( -type f -or -type l \) -print ) | grep -vE "$EXCLUDES" | sed 's/^.//' >> tmp/conf.files
        cp -R $config_dir/* $RPM_BUILD_ROOT/
    fi
done

cp %{dhd_path}/configs/hybris.conf $RPM_BUILD_ROOT/%{_sharedstatedir}/environment/nemo/99-hybris.conf

# SSU board mapping for hardware adaptation
mkdir -p $RPM_BUILD_ROOT/%{board_mapping_dir}
sed -e 's|%DEVICE%|%{device}|g' \
    -e 's|%VENDOR%|%{vendor}|g' \
    -e "s|%DEVICE_PRETTY%|%{device_pretty}|g" \
    -e "s|%VENDOR_PRETTY%|%{vendor_pretty}|g" \
    -e "s|%ARCH%|%{__isa_name}|g" \
    %{dhd_path}/ssu/board-mappings.d/05-sfe.ini.in >$RPM_BUILD_ROOT/%{board_mapping_file}

# Copy kickstart packs (for %%{device}-kickstart-configuration)
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/ssu/kickstart/pack/%{device}
sed -e 's|@DEVICE@|%{device}|g' %{dhd_path}/kickstart/pack/package-droid-updater > $RPM_BUILD_ROOT/%{_datadir}/ssu/kickstart/pack/%{device}/package-droid-updater
# and attachments
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/ssu/kickstart/attachment
cp %{dhd_path}/kickstart/attachment/hybris $RPM_BUILD_ROOT/%{_datadir}/ssu/kickstart/attachment/%{device}

# Keyboard map file
/usr/lib/qt5/bin/kmap2qmap %{dhd_path}/configs/droid.kmap %{dhd_path}/configs/us.kmap tmp/droid.qmap
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/qt5/keymaps
cp tmp/droid.qmap $RPM_BUILD_ROOT/%{_datadir}/qt5/keymaps/droid.qmap

# hw-release
cp tmp/hw-release %{buildroot}/%{_sysconfdir}/hw-release 
# for use in the -devel package
cp tmp/hw-release %{buildroot}/%{_libdir}/droid-devel/hw-release.vars

# Screen rotation dconf configuration
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/dconf/db/vendor.d/locks
cp %{dhd_path}/dconf/screen-rotation.txt $RPM_BUILD_ROOT/%{_sysconfdir}/dconf/db/vendor.d/screen-rotation.txt
cp %{dhd_path}/dconf/locks-screen-rotation.txt $RPM_BUILD_ROOT/%{_sysconfdir}/dconf/db/vendor.d/locks/screen-rotation.txt

# This ghost file must exist in the installroot
touch $RPM_BUILD_ROOT/%{_libdir}/droid/droid-user-remove.sh.installed

# Kernel and module installation; to
# /boot and modules to /lib as normal
KERNEL_RELEASE=$(cat out/target/product/%{device}/obj/*/include/config/kernel.release)
cp out/target/product/%{device}/kernel $RPM_BUILD_ROOT/boot/kernel-$KERNEL_RELEASE
MOD_DIR=$RPM_BUILD_ROOT/lib/modules/$KERNEL_RELEASE
mkdir -p $MOD_DIR
cp -a out/target/product/%{device}/system/lib/modules/. $MOD_DIR/. || true
cp -a out/target/product/%{device}/obj/*/modules.builtin $MOD_DIR/. || true
cp -a out/target/product/%{device}/obj/*/modules.order $MOD_DIR/. || true

# Images are installed to /boot - they'll probably be unpacked using
# rpm2cpio mostly anyhow
cp out/target/product/%{device}/hybris-boot.img $RPM_BUILD_ROOT/boot/
cp out/target/product/%{device}/hybris-recovery.img $RPM_BUILD_ROOT/boot/


# Everything is installed; get a list of the units we installed to
# allow the systemd_post to work... and then install that:
echo "$(cd $RPM_BUILD_ROOT%{_unitdir}; echo *)" > tmp/units/all-units.txt
install -D tmp/units/all-units.txt $RPM_BUILD_ROOT%{_libdir}/droid/all-units.txt

/usr/bin/repomd-pattern-builder.py --patternxml -p ./rpm/patterns/%{device} -o %{buildroot}/usr/share/package-groups/ --version=%{version} --release=%{release}

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
# To add additional groups define a HA config macro like android_config

%post sailfish-config

%{_bindir}/add-oneshot dconf-update || :

%post kernel-modules
# This runs on the device at install or in mic chroot at img build
# in chroot the kernel version is not known.
for ver in $(cd /lib/modules; echo *); do
  /sbin/depmod $ver || :
done

%if 0%{?enable_kernel_update:1}
%post img-boot
# This oneshot is enabled only during package upgrades, as initial
# installation is done when we flash device.
if [ $1 -ne 1 ] ; then
  add-preinit-oneshot /var/lib/platform-updates/flash-bootimg.sh
fi
%endif

################################################################
# Begin files section

%files -f tmp/conf.files
%defattr(-,root,root,-)
/sbin/*
%{_bindir}/droid/droid-init-done.sh
%{_bindir}/droid/kill-cgroup.sh
%{_bindir}/droid/droid-hal-startup.sh
# This binary should probably move to /sbin/
%{_libdir}/droid/droid-user-add.sh
%{_libdir}/droid/droid-user-remove.sh
%{_libdir}/droid/all-units.txt
%{board_mapping_file}
# Created in %%post
# init-debug
%attr(755,root,root) /init-debug
%ghost %attr(755, root, root) %{_libdir}/droid/droid-user-remove.sh.installed
# droid binaries
%{_libexecdir}/droid-hybris/system/bin/

# Non executable files
%defattr(644,root,root,-)
%{_unitdir}
# hybris and /dev/alog/ libraries
%{_libexecdir}/droid-hybris/system/lib/
# just /dev/alog/ libraries (for trying to run pure android apps)
%{_libexecdir}/droid-hybris/lib-dev-alog/.
/lib/udev/rules.d/*
/lib/udev/platform-device
# Droid config droppings
/*.rc
/default.prop
%{_sharedstatedir}/environment/compositor/droid-hal-device.conf
%{_sharedstatedir}/environment/nemo/99-hybris.conf
%{_datadir}/qt5/keymaps/droid.qmap
# Hardware and build info
%config %{_sysconfdir}/hw-release
# Disabling v4l.rules
%{_sysconfdir}/udev/rules.d/60-persistent-v4l.rules

%files devel
%defattr(644,root,root,-)
%{_libdir}/droid-devel/
%{_libdir}/pkgconfig/*pc

%files sailfish-config
%defattr(-,root,root,-)
%{_sysconfdir}/dconf/db/vendor.d/screen-rotation.txt
%{_sysconfdir}/dconf/db/vendor.d/locks/screen-rotation.txt

%files kernel
%defattr(644,root,root,-)
/boot/kernel*

%files kernel-modules
%defattr(644,root,root,-)
/lib/modules

%files img-boot
%defattr(644,root,root,-)
/boot/hybris-boot.img
/boot/update-binary
/boot/hybris-updater-script
/boot/hybris-updater-unpack.sh
%if 0%{?enable_kernel_update:1}
%attr(744, root, root) /var/lib/platform-updates/flash-bootimg.sh
%endif

%files img-recovery
%defattr(644,root,root,-)
/boot/hybris-recovery.img

%files pulseaudio-settings
%defattr(644,root,root,-)

%files policy-settings
%defattr(644,root,root,-)

%files kickstart-configuration
%defattr(644,root,root-)
%{_datadir}/ssu/kickstart/pack/%{device}/package-droid-updater
%{_datadir}/ssu/kickstart/attachment/%{device}

%files patterns
%defattr(-,root,root,-)
%{_datadir}/package-groups/*.xml

%files preinit-plugin
%defattr(-,root,root,-)
%{_libdir}/startup/preinit/*

