%define strip /bin/true
%define __requires_exclude  ^.*$
%define __find_requires     %{nil}
%global debug_package       %{nil}
%define __provides_exclude_from ^.*$
%define device_rpm_architecture_string armv7hl
%define _target_cpu %{device_rpm_architecture_string}


Name:          audioflingerglue
Summary:       Android AudioFlinger glue library
Version:       0.0.0
Release:       1
Group:         System/Libraries
License:       ASL 2.0
BuildRequires: tar
Source0:       %{name}-%{version}.tgz
AutoReqProv:   no

%description
%{summary}

%package       devel
Summary:       audioflingerglue development headers
Group:         System/Libraries
Requires:      audioflingerglue = %{version}-%{release}
BuildArch:     noarch

%description   devel
%{summary}

%prep

%if %{?device_rpm_architecture_string:0}%{!?device_rpm_architecture_string:1}
echo "device_rpm_architecture_string is not defined"
exit -1
%endif

%setup

%build
pwd
ls 
tar -xvf %name-%version.tgz
%install

if [ -f out/target/product/*/system/lib64/libaudioflingerglue.so ]; then
DROIDLIB=lib64
else
DROIDLIB=lib
fi

mkdir -p $RPM_BUILD_ROOT/%{_libexecdir}/droid-hybris/system/$DROIDLIB/
mkdir -p $RPM_BUILD_ROOT/%{_libexecdir}/droid-hybris/system/bin/
mkdir -p $RPM_BUILD_ROOT/%{_includedir}/audioflingerglue/
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/audioflingerglue/
pushd %name-%version
cp out/target/product/*/system/$DROIDLIB/libaudioflingerglue.so \
    $RPM_BUILD_ROOT/%{_libexecdir}/droid-hybris/system/$DROIDLIB/

cp out/target/product/*/system/bin/miniafservice \
    $RPM_BUILD_ROOT/%{_libexecdir}/droid-hybris/system/bin/

cp external/audioflingerglue/*.h $RPM_BUILD_ROOT/%{_includedir}/audioflingerglue/
sed -e "s/@TARGET_LIB_ARCH@/$DROIDLIB/" external/audioflingerglue/hybris.c.in > \
    $RPM_BUILD_ROOT/%{_datadir}/audioflingerglue/hybris.c

popd

LIBAFSOLOC=$RPM_BUILD_ROOT/file.list
echo %{_libexecdir}/droid-hybris/system/$DROIDLIB/libaudioflingerglue.so > %{LIBAFSOLOC}
 
%files -f %{LIBAFSOLOC}
%defattr(-,root,root,-)
%{_libexecdir}/droid-hybris/system/bin/miniafservice

%files devel
%defattr(-,root,root,-)
%{_includedir}/audioflingerglue/*.h
%{_datadir}/audioflingerglue/hybris.c

