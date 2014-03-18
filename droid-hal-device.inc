# This file should be %included into a device specific spec file
# where macros are defined:
# device: should be the CM codename 
# vendor: determine the directory used for ./device/<vendor>/<device>

%define android_root .

Summary: 	Droid HAL package for %{device}
License: 	BSD-3-Clause
Name: 		droid-hal-%{device}
Version: 	0.0.6
Release: 	0
Provides:       droid-hal
Source0: 	%{name}-%{version}.tar.bz2
Source1: 	makefstab
Source2: 	usergroupgen.c
Source3:        makeudev
Source4:        apply-permissions.c
Source5:        makefile
Source7:        device-%{vendor}-%{device}-configs
Source8:        hybris.conf
Source9:        sailfish-oneshot-rotation
Source10:       droid-hal-device.inc
# This is copied from libhybris and should be kept in-sync:
Source11:	extract-headers.sh
Source12:       init-debug
Source13:       device-configs-all
# Allow device specific sources to be defined using dhd_sources
# Reserve Source50 onwards
%{?dhd_sources}

Group:		System
#BuildArch:	noarch
# To provide systemd services and udev rules
Requires:       droid-system-packager
# Note that oneshot is not in mer-core (yet)
BuildRequires:  oneshot
BuildRequires:  mer-kernel-checks
BuildRequires:  systemd
%systemd_requires
%{_oneshot_requires_post}
# Should be removed when usb-moded is fixed and android-usb.services removed
Conflicts: usb-moded

%description
%{summary}.

%package devel
Group:	Development/Tools
Requires: %{name} = %{version}-%{release}
Provides: droid-hal-devel
Summary: Development files for droid-hal device: %{device}

%description devel
Device specific droid headers for %{device}.
Needed by libhybris

%package sailfish-config
Provides: droid-hal-sailfish-config
Group:	System
BuildArch:	noarch
Requires: %{name} = %{version}-%{release}
Requires: oneshot
Summary: Per device configuration for sailfish for droid-hal device: %{device}

%description sailfish-config
Configure sailfish eg naturally landscape devices like mako

%package kernel
Provides: droid-hal-kernel
Group:	System
Summary: Kernel for droid-hal device: %{device}

%description kernel
Just the kernel - mainly useful if you want to make a custom img

%package kernel-modules
Provides: droid-hal-kernel-modules
Group:	System
Summary: Kernel modules for droid-hal device: %{device}

%description kernel-modules
Just the kernel modules

%package img-boot
Provides: droid-hal-img-boot
Group:	System
Summary: Boot img for droid-hal device: %{device}

%description img-boot
The boot.img for device

%package img-recovery
Provides: droid-hal-img-recovery
Group:	System
BuildArch:	noarch
Summary: Recovery image for droid-hal device: %{device}

%description img-recovery
The recovery.img for device

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

%prep
%setup -q

%build
%if %{without mer_verify_kernel_config}
echo Verifying kernel config
mer_verify_kernel_config \
    %{android_root}/out/target/product/%{device}/obj/KERNEL_OBJ/.config
%endif

echo Building local tools
rm -rf rpmsrc
mkdir rpmsrc
cp %{SOURCE2} %{SOURCE4} rpmsrc/
ANDROID_ROOT=$(readlink -e %{android_root})
(cd rpmsrc; make ANDROID_ROOT=$ANDROID_ROOT -f %{SOURCE5})

echo Building uid scripts
rpmsrc/usergroupgen add > rpmsrc/droid-user-add.sh
rpmsrc/usergroupgen remove > rpmsrc/droid-user-remove.sh

echo Building udev rules
rm -rf udev.rules
mkdir udev.rules
# Device specific ueventd rules is the "not goldfish" one
%{SOURCE3} \
    %{android_root}/out/target/product/%{device}/root/ueventd.rc \
    $(ls %{android_root}/out/target/product/%{device}/root/ueventd.*.rc | grep -v .goldfish.rc) \
        > udev.rules/999-android-system.rules

echo Building mount units
rm -rf units
mkdir -p units
# Use the makefstab and tell it what mountpoints to skip. It will
# generate .mount units which will be part of local-fs.target
# skip /cache and /data - won't touch them (yet)
cat %{android_root}/out/target/product/%{device}/root/fstab.* | (cd units; %{SOURCE1} /boot /cache /data /misc /recovery /staging /storage/usbdisk )

echo Fixing up mount points
hybris/hybris-boot/fixup-mountpoints %{device} units/*

echo "$(cd units; echo *)" > units/mount-units.txt

%install
echo install $(cat units/mount-units.txt)
rm -rf $RPM_BUILD_ROOT
# Create dir structure
mkdir -p $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/lib-dev-alog/
mkdir -p $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system
mkdir -p $RPM_BUILD_ROOT%{_libdir}/droid
mkdir -p $RPM_BUILD_ROOT%{_libdir}/droid-devel/
mkdir -p $RPM_BUILD_ROOT%{_unitdir}
mkdir -p $RPM_BUILD_ROOT/lib/udev/rules.d
mkdir -p $RPM_BUILD_ROOT/%{_sharedstatedir}/environment/compositor
mkdir -p $RPM_BUILD_ROOT/%{_sharedstatedir}/environment/nemo
mkdir -p $RPM_BUILD_ROOT/%{_oneshotdir}

mkdir -p $RPM_BUILD_ROOT%{_libdir}/modules/
mkdir -p $RPM_BUILD_ROOT%{_libdir}/droid

mkdir -p $RPM_BUILD_ROOT/img
mkdir -p $RPM_BUILD_ROOT/boot
mkdir -p $RPM_BUILD_ROOT/lib/modules

# Install
cp -a %{android_root}/out/target/product/%{device}/system/bin/updater $RPM_BUILD_ROOT/boot/update-binary
cp -a %{android_root}/out/target/product/%{device}/hybris-updater-script $RPM_BUILD_ROOT/boot
cp -a %{android_root}/out/target/product/%{device}/hybris-updater-unpack.sh $RPM_BUILD_ROOT/boot

cp -a %{android_root}/out/target/product/%{device}/root/. $RPM_BUILD_ROOT/
cp -a %{android_root}/out/target/product/%{device}/system/{bin,lib} $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/system/.
cp -a %{android_root}/out/target/product/%{device}/obj/{lib,include} $RPM_BUILD_ROOT%{_libdir}/droid-devel/
cp -a %{android_root}/out/target/product/%{device}/symbols $RPM_BUILD_ROOT%{_libdir}/droid-devel/

# Run extract-headers.sh
HDRS=$RPM_BUILD_ROOT%{_libdir}/droid-devel/droid-headers
echo "Extracting headers for hybris"
%SOURCE11 -p %{_libdir}/droid-devel/droid-headers . $HDRS/ > /dev/null

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
# grep -l dev/alog %{android_root}/out/target/product/%{device}/system/lib/*
# libdsyscalls.so and libc.so are blacklisted
ln -s ../system/lib/{liblog.so,libcutils.so} $RPM_BUILD_ROOT%{_libexecdir}/droid-hybris/lib-dev-alog/.

cp -a units/*.mount $RPM_BUILD_ROOT/%{_unitdir}

# Install the udev rules and supporting script
cp -a udev.rules/* $RPM_BUILD_ROOT/lib/udev/rules.d/

# Install init-debug which will provide usb access and non-blocking telnet
cp -a %{SOURCE12} $RPM_BUILD_ROOT/

# droid user support This may be better done by passing a list of
# users/groups and running 'ensure_usergroups_exist newlist oldlist'
# which would preserve oldlist in %post and delete any users no longer
# needed (unlikely!). This avoids the transient removal of uids and
# group issues
install -D rpmsrc/droid-user-add.sh $RPM_BUILD_ROOT%{_libdir}/droid/droid-user-add.sh
install -D rpmsrc/droid-user-remove.sh $RPM_BUILD_ROOT%{_libdir}/droid/droid-user-remove.sh

# droid permission fixer
install -D rpmsrc/apply-permissions $RPM_BUILD_ROOT%{_libdir}/droid/apply-permissions

# the list of mount units
install -D units/mount-units.txt $RPM_BUILD_ROOT%{_libdir}/droid/mount-units.txt

# Remove cruft
rm $RPM_BUILD_ROOT/fstab.*
rm -rf $RPM_BUILD_ROOT/{proc,sys,dev,sepolicy} $RPM_BUILD_ROOT/{file,property,seapp}_contexts
rm -rf $RPM_BUILD_ROOT/{charger,res,data}

# Name this so droid-system-packager's droid-hal-startup.sh can find it
mkdir -p $RPM_BUILD_ROOT/sbin
mv $RPM_BUILD_ROOT/init $RPM_BUILD_ROOT/sbin/droid-hal-init
# Rename any symlinks to droid's /init 
find $RPM_BUILD_ROOT/sbin/ -lname ../init -execdir echo rm {} \; -execdir echo "ln -s" ./droid-hal-init {} \;
#mv $RPM_BUILD_ROOT/charger $RPM_BUILD_ROOT/sbin/droid-hal-charger

# To set the environment for Qt/hybris (note this is a bit lipstick specific)
echo -n "" > rpmsrc/conf.files
# Install files from device-configs-all (if any)
for config_dir in %{SOURCE7} %{SOURCE13}; do
    if [ -d $config_dir ]; then
        (cd $config_dir; find . \( -type f -or -type l \) -print ) | sed 's/^.//' >> rpmsrc/conf.files
        cp -R $config_dir/* $RPM_BUILD_ROOT/
    fi
done

cp %{SOURCE8} $RPM_BUILD_ROOT/%{_sharedstatedir}/environment/nemo/99-hybris.conf

# Add the oneshot
cp %{SOURCE9} $RPM_BUILD_ROOT/%{_oneshotdir}/oneshot-rotation

# This ghost file must exist in the installroot
touch $RPM_BUILD_ROOT/%{_libdir}/droid/droid-user-remove.sh.installed

# Kernel and module installation; to
# /boot and modules to /lib as normal
KERNEL_RELEASE=$(cat out/target/product/%{device}/obj/KERNEL_OBJ/include/config/kernel.release)
cp out/target/product/%{device}/kernel $RPM_BUILD_ROOT/boot/kernel-$KERNEL_RELEASE
MOD_DIR=$RPM_BUILD_ROOT/lib/modules/$KERNEL_RELEASE
mkdir -p $MOD_DIR
cp -a out/target/product/%{device}/system/lib/modules/. $MOD_DIR/.
cp -a out/target/product/%{device}/system/lib/modules/. $MOD_DIR/.
cp -a out/target/product/%{device}/obj/KERNEL_OBJ/modules.builtin $MOD_DIR/. || true
cp -a out/target/product/%{device}/obj/KERNEL_OBJ/modules.order $MOD_DIR/. || true

# Images are installed to /boot - they'll probably be unpacked using
# rpm2cpio mostly anyhow
cp out/target/product/%{device}/hybris-boot.img $RPM_BUILD_ROOT/boot/
cp out/target/product/%{device}/hybris-recovery.img $RPM_BUILD_ROOT/boot/

%preun
for u in $(cat %{_libdir}/droid/mount-units.txt); do
%systemd_preun $u
done
# Only run this during final cleanup
if [ $1 == 0 ]; then
    echo purging old droid users and groups
    %{_libdir}/droid/droid-user-remove.sh.installed
    true
fi

%post
for u in $(cat %{_libdir}/droid/mount-units.txt); do
%systemd_post $u
done
cd %{_libdir}/droid
# Upgrade: remove users using stored file, then add new ones
if [ $1 == 2 ]; then
    # Remove installed users (at this point droid-user-remove.sh
    # refers to the new set of UIDs)
    echo removing old droid users and groups
    ./droid-user-remove.sh.installed
fi
# Now for both install/update add the users and force-store a removal file
echo creating droid users and groups
./droid-user-add.sh
cp -f droid-user-remove.sh droid-user-remove.sh.installed

# Now ensure default user has access to various subsystems this HA provides
# These are the default android ones:
/usr/bin/groupadd-user graphics
/usr/bin/groupadd-user system
/usr/bin/groupadd-user input
/usr/bin/groupadd-user camera
# To add additional groups define a HA config macro like android_config

%post sailfish-config

if [ "$1" -eq 1 ]; then
    echo Adding oneshot
    %{_bindir}/add-oneshot --user oneshot-rotation
fi

%post kernel-modules
# This runs on the device at install or in mic chroot at img build
# in chroot the kernel version is not known.
for ver in $(cd /lib/modules; echo *); do
  depmod $ver
done

%files -f rpmsrc/conf.files
%defattr(-,root,root,-)
/sbin/*
# This binary should probably move to /sbin/
%{_libdir}/droid/droid-user-add.sh
%{_libdir}/droid/droid-user-remove.sh
%{_libdir}/droid/apply-permissions
%{_libdir}/droid/mount-units.txt
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
# Droid config droppings
/*.rc
/default.prop
%{_sharedstatedir}/environment/compositor/droid-hal-device.conf
%{_sharedstatedir}/environment/nemo/99-hybris.conf

%files devel
%defattr(644,root,root,-)
%{_libdir}/droid-devel/
%{_libdir}/pkgconfig/*pc

%files sailfish-config
%defattr(-,root,root,-)
%attr(755,root,root) %{_oneshotdir}/oneshot-rotation

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

%files img-recovery
%defattr(644,root,root,-)
/boot/hybris-recovery.img

%files pulseaudio-settings
%defattr(644,root,root,-)

%files policy-settings
%defattr(644,root,root,-)

