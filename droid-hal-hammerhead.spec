# device is the cyanogenmod codename for the device
# eg mako = Nexus 4
%define device hammerhead
# vendor is used in device/%vendor/%device/
%define vendor lge

# Manufacturer and device name to be shown in UI
%define vendor_pretty LG
%define device_pretty Nexus 5

%define enable_kernel_update 1

# WARNING: If you comment a macro, it will still be picked up by rpmbuild!
# The only proper way to disable a macro is:
#define have_gstdroid 1
# ^ also 1 or 0 still means macro is defined when checking in conditionals!
# When you want to enable it. simply replace '#' with '%' in front of 'define'

Requires: rfkill
Requires: bluez >= 4.101+git33
# Each device that provides files in /etc/ofono should have this provides
Provides:	ofono-configs

%include rpm/droid-hal-device.inc

