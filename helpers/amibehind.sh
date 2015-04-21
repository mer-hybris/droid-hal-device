#!/bin/bash
# checker script to see if anything can be updated
# Copyright (c) 2015 Jolla Ltd.
# Contact: Simonas Leleiva <simonas.leleiva@jollamobile.com>

if [ -z $DEVICE ]; then
    echo 'Error: $DEVICE is undefined. Please run hadk'
    exit 1
fi
if [ ! -d rpm/dhd ]; then
    echo $0: launch this script from the $ANDROID_ROOT directory
    exit 1
fi

# A POSIX variable
OPTIND=1         # Reset in case getopts has been used previously in the shell.
CONFIG_DIR=hybris/droid-configs

while getopts "ps" opt; do
    case "$opt" in
    p)  rpm/dhd/helpers/add_new_device.sh -y
        git --no-pager --git-dir=$CONFIG_DIR/.git --work-tree=$CONFIG_DIR diff
        echo
        echo "Patterns updated. Do cd $CONFIG_DIR/ and commit changes you're happy with"
        ;;
    s)  echo Comparing submodules currently unsupported
        ;;
    esac
done

