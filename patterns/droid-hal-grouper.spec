#FIXME: Disable mer kernel config verification for the time being.
%bcond_without mer_verify_kernel_config

# device is the cyanogenmod codename for the device
# eg mako = Nexus 4
#    grouper = Nexus 7 (2012)
%define device grouper
# vendor is used in device/%vendor/%device/
%define vendor asus

# Manufacturer and device name to be shown in UI
%define vendor_pretty Asus
%define device_pretty Nexus 7 (2012)

%include rpm/droid-hal-device.inc

