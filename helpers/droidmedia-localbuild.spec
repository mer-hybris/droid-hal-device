%define strip /bin/true
%define __requires_exclude  ^.*$
%define __find_requires     %{nil}
%global debug_package       %{nil}
%define __provides_exclude_from ^.*$
%define device_rpm_architecture_string @PORT_ARCH@
%define _target_cpu %{device_rpm_architecture_string}

Name:          droidmedia
Summary:       Android media wrapper library
Version:       0.0.0
Release:       1
Group:         System/Libraries
License:       ASL 2.0
BuildRequires: tar
#BuildRequires: ubu-trusty
#BuildRequires: sudo-for-abuild
#BuildRequires: droid-bin-src-full
Source0:       %{name}-%{version}.tgz
AutoReqProv:   no

%description
%{summary}

%prep

%if %{?device_rpm_architecture_string:0}%{!?device_rpm_architecture_string:1}
echo "device_rpm_architecture_string is not defined"
exit -1
%endif

%setup 

#sudo chown -R abuild:abuild /home/abuild/src/droid/
#mv /home/abuild/src/droid/* .
#mkdir -p external
#pushd external
#tar -zxf %SOURCE0
#mv droidmedia* droidmedia
#popd
%build
pwd
ls 
tar -xvf %name-%version.tgz
%install

pushd %name-%version
if [ -f out/target/product/@DEVICE@/system/lib64/libdroidmedia.so ]; then
DROIDLIB=lib64
else
DROIDLIB=lib
fi

mkdir -p $RPM_BUILD_ROOT/%{_libexecdir}/droid-hybris/system/$DROIDLIB/
mkdir -p $RPM_BUILD_ROOT/%{_libexecdir}/droid-hybris/system/bin/
mkdir -p $RPM_BUILD_ROOT/%{_includedir}/droidmedia/
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/droidmedia/
cp out/target/product/@DEVICE@/system/$DROIDLIB/libdroidmedia.so \
    $RPM_BUILD_ROOT/%{_libexecdir}/droid-hybris/system/$DROIDLIB/

cp out/target/product/@DEVICE@/system/$DROIDLIB/libminisf.so \
    $RPM_BUILD_ROOT/%{_libexecdir}/droid-hybris/system/$DROIDLIB/

cp out/target/product/@DEVICE@/system/bin/minimediaservice \
    $RPM_BUILD_ROOT/%{_libexecdir}/droid-hybris/system/bin/

cp out/target/product/@DEVICE@/system/bin/minisfservice \
    $RPM_BUILD_ROOT/%{_libexecdir}/droid-hybris/system/bin/

mkdir -p $RPM_BUILD_ROOT/%{_libexecdir}/droid-hybris/system/etc/init/
cp external/droidmedia/init/*.rc \
   $RPM_BUILD_ROOT/%{_libexecdir}/droid-hybris/system/etc/init/

popd

LIBDMSOLOC=file.list
echo %{_libexecdir}/droid-hybris/system/$DROIDLIB/libdroidmedia.so >> ${LIBDMSOLOC}
echo %{_libexecdir}/droid-hybris/system/$DROIDLIB/libminisf.so >> ${LIBDMSOLOC}

%files -f file.list
%defattr(-,root,root,-)
%{_libexecdir}/droid-hybris/system/etc/init/*.rc
%{_libexecdir}/droid-hybris/system/bin/minimediaservice
%{_libexecdir}/droid-hybris/system/bin/minisfservice

