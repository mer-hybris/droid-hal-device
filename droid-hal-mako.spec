# device is the cyanogenmod codename for the device
# eg mako = Nexus 4
%define device mako
Summary: 	Droid HAL package
License: 	BSD-3-Clause
Name: 		droid-hal-%{device}
Version: 	0.0.1
Release: 	0
Source0: 	%{name}-%{version}.tar.bz2
Source1: 	makefstab
Group:		System
#BuildArch:	noarch
BuildRequires:  systemd
%systemd_requires

%description
%{summary}.

%package devel
Group:	Development/Tools
Requires: %{name} = %{version}-%{release}
Summary: Development files for droid hal

%description devel
%{summary}.

%prep
%setup -q

%build
echo Building mount units
rm -rf units
mkdir units
cd units
# Use the makefstab and tell it what mountpoints to skip. It will
# generate .mount units which will be part of local-fs.target
%{SOURCE1} /system /cache /data < ../device/lge/mako/fstab.mako 

# This is broken pending systemd > 191-2 so hack the generated unit files :(
# See: https://bugzilla.redhat.com/show_bug.cgi?id=859297
sed -i 's block/platform/msm_sdcc.1/by-name/modem mmcblk0p1 ' *mount
sed -i 's block/platform/msm_sdcc.1/by-name/persist mmcblk0p2 ' *mount


%define units %(cd units;echo *)

%install
echo install %units
rm -rf $RPM_BUILD_ROOT
# Create dir structure
mkdir -p $RPM_BUILD_ROOT/system
mkdir -p $RPM_BUILD_ROOT/usr/lib/droid-devel/
mkdir -p $RPM_BUILD_ROOT/etc/droid-init/
mkdir -p $RPM_BUILD_ROOT/%{_unitdir}

# Install
cp -a out/target/product/%{device}/root/. $RPM_BUILD_ROOT/
cp -a out/target/product/%{device}/system/. $RPM_BUILD_ROOT/system/.
cp -a out/target/product/%{device}/obj/{lib,include} $RPM_BUILD_ROOT/usr/lib/droid-devel/
cp -a out/target/product/%{device}/symbols $RPM_BUILD_ROOT/usr/lib/droid-devel/

cp -a units/* $RPM_BUILD_ROOT/%{_unitdir}

# Remove cruft
rm $RPM_BUILD_ROOT/fstab.*
rmdir $RPM_BUILD_ROOT/{proc,sys,dev}

# Relocate rc files and other things left in / where possible
# mv $RPM_BUILD_ROOT/*rc $RPM_BUILD_ROOT/etc/droid-init/
# Name this so droid-system-packager's droid-hal-startup.sh can find it
mv $RPM_BUILD_ROOT/init $RPM_BUILD_ROOT/sbin/droid-hal-init
# Rename any symlinks to droid's /init 
find $RPM_BUILD_ROOT/sbin/ -lname ../init -execdir echo rm {} \; -execdir echo "ln -s" ./droid-hal-init {} \;
#mv $RPM_BUILD_ROOT/charger $RPM_BUILD_ROOT/sbin/droid-hal-charger
%preun
for u in %units; do
%systemd_preun $u
done

%post
for u in %units; do
%systemd_post $u
done

%files
%defattr(-,root,root,-)
# Standard droid paths
/system/
/res
/data
/sbin/*
# move the .rc files to %%{_sysconfdir}/droid-init if possible
%attr(644, root, root) /*.rc
# Can this move?
%attr(644, root, root) /default.prop
# This binary should probably move to /sbin/
/charger
%{_unitdir}

%files devel
%defattr(-,root,root,-)
%{_libdir}/droid-devel/

