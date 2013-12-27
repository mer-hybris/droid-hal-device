#FIXME: Disable mer kernel config verification for the time being.
%bcond_without mer_verify_kernel_config

# device is the cyanogenmod codename for the device
# eg mako = Nexus 4
#    grouper = Nexus 7 (2012)
%define device grouper
# vendor is used in device/%vendor/%device/
%define vendor asus

%include rpm/droid-hal-device.inc

