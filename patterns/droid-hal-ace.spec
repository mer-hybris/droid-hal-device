# device is the cyanogenmod codename for the device
# eg mako = Nexus 4
%define device ace
# vendor is used in device/%vendor/%device/
%define vendor htc

# Manufacturer and device name to be shown in UI
%define vendor_pretty HTC
%define device_pretty Desire HD

%define enable_kernel_update 0

%include rpm/droid-hal-device.inc
