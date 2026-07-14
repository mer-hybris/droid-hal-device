#!/bin/sh -e
# build kernel - Script to build kernel from android source tree
# used for Android versions where in tree builds aren't possible.
# Inspired by Sonys build_kernels_gcc.sh script.

mkdtimg=$ANDROID_ROOT/out/host/linux-x86/bin/mkdtimg
avbtool=$ANDROID_ROOT/out/host/linux-x86/bin/avbtool
droid_target_dir="$ANDROID_ROOT"/out/target/product/${HABUILD_DEVICE:-$DEVICE}
kernel_build_dir=$droid_target_dir/obj/KERNEL_OBJ
kernel_target_file=$droid_target_dir/kernel
kernel_build()
{
    (
        cd "$KERNEL_SOURCE" || exit 1
        # shellcheck disable=SC2046
        # NOTE: the output of nproc won't contain spaces
        make O="$kernel_build_dir" ARCH=arm64 CROSS_COMPILE="$CROSS_COMPILE" CROSS_COMPILE_ARM32="$CROSS_COMPILE_ARM32" -j$(nproc) "$@"
    )
}

# Script specific env variables:
# CROSS_COMPILE - Path to cross compiler
# CROSS_COMPILE_ARM32 - Path to cross compiler (32bit)
# KERNEL_DEFCONFIG - defconfig used to generate .config something like aosp_$platform_$HABUILD_DEVICE
# KERNEL_SOURCE - Path to kernel source
# DROID_TARGET_KERNEL_ARCH - Target architecture of device like droid_target_arch but different naming
# DROID_TARGET_KERNEL_DTB - If defined dtbo.img gets generated if your device needs it
for env_var in CROSS_COMPILE CROSS_COMPILE_ARM32 KERNEL_DEFCONFIG KERNEL_SOURCE VENDOR DEVICE ANDROID_ROOT DROID_TARGET_KERNEL_ARCH ; do
    if ! eval "[ \$$env_var ]" ; then
        echo "$env_var not defined, check env" >&2
        var_missing=true
    fi
done
[ $var_missing ] && exit 1

mkdir -p "$kernel_build_dir"



kernel_build $KERNEL_DEFCONFIG
kernel_build

cp "$kernel_build_dir/arch/$DROID_TARGET_KERNEL_ARCH/boot/Image.gz-dtb" "$kernel_target_file"

if [ "$DROID_TARGET_KERNEL_DTB" ] ; then

    # Check if mkdtimg tool exists
    if [ ! -f "$mkdtimg" ]; then
        echo "mkdtimg: File not found!" >&2
        echo "Building mkdtimg" >&2
        # Only build the required tools for building the dtbo image and nothing more
        export ALLOW_MISSING_DEPENDENCIES=true
        make mkdtimg verity_key
    fi

    "$mkdtimg" create "$droid_target_dir"/dtbo.img "$(find "$kernel_build_dir/arch/$DROID_TARGET_KERNEL_ARCH/boot/dts" -name "*.dtbo")"
    "$avbtool" add_hash_footer --image "$droid_target_dir"/dtbo.img --partition_size 8388608 --partition_name dtbo
fi
