# device is the cyanogenmod codename for the device
# eg mako = Nexus 4
%define device i9305
# vendor is used in device/%vendor/%device/
%define vendor samsung

# android_config is the set of #defines needed by libhybris builds to
# be injected into android_config.h
# This could eventually be obtained by parsing the BoardConfig.mk
%define android_config \
#define EXYNOS4_ENHANCEMENTS 1\
%{nil}

%include rpm/droid-hal-device.inc
