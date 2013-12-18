# device is the cyanogenmod codename for the device
# eg mako = Nexus 4
%define device i9305
# vendor is used in device/%vendor/%device/
%define vendor samsung

%define android_config \
#define EXYNOS4_ENHANCEMENTS 1\
%{nil}

%include rpm/droid-hal-device.inc
