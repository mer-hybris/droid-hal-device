# device is the cyanogenmod codename for the device
# eg mako = Nexus 4
%define device mako
# vendor is used in device/%vendor/%device/
%define vendor lge

# Manufacturer and device name to be shown in UI
%define vendor_pretty LG
%define device_pretty Nexus 4
%define android_config \
#define QCOM_BSP 1\
%{nil}
%include rpm/droid-hal-device.inc
