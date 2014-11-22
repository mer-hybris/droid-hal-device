#FIXME: Disable mer kernel config verification for the time being.
%bcond_without mer_verify_kernel_config

# device is the cyanogenmod codename for the device
# eg mako = Nexus 4
#    grouper = Nexus 7 (2012)
# bacon = One (OnePlus) (2014)
%define device bacon
# vendor is used in device/%vendor/%device/
%define vendor oneplus

# Manufacturer and device name to be shown in UI
%define vendor_pretty OnePlus
%define device_pretty One (2014)

%include rpm/droid-hal-device.inc

