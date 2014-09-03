# device is the cyanogenmod codename for the device
# eg mako = Nexus 4
%define device maguro
# vendor is used in device/%vendor/%device/
%define vendor samsung

# Manufacturer and device name to be shown in UI
%define vendor_pretty Samsung
%define device_pretty Galaxy Nexus

%define enable_kernel_update 1

%include rpm/droid-hal-device.inc
