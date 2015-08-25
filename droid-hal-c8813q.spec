# device is the cyanogenmod codename for the device
# eg mako is Nexus 4
%define device c8813q
# vendor is used in device/huawei/c8813q/
%define vendor huawei
# Manufacturer and device name to be shown in UI
%define vendor_pretty Huawei
%define device_pretty Ascend c8813q/g525-00
%define android_config \
#define QCOM_BSP 1\
%{nil}
%include rpm/droid-hal-device.inc
