########################
# Copyright(c) 2014-2015 DongGuan HeWei Communication Technologies Co. Ltd.
# file    mount-SK.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    12Jul15
########################
#!/bin/bash
if [ -e /usr/sbin/fakeiOSDevice ];then
	echo "FakeiOSDevice not support change to USB mass storage" >> /dev/console
	exit 0
elif [ -e /usr/sbin/fakeCarLifeDevice ];then
	echo "FakeCarLifeDevice not support change to USB mass storage" >> /dev/console
	exit 0
fi
usb_dev_path=/dev/sda
if [ $# -gt 0 ];then
	usb_dev_path=$1
fi
echo 0 > /sys/class/android_usb_accessory/android0/enable
umount -l /mnt/UPAN
echo "$usb_dev_path" >  /sys/devices/soc0/soc.0/2100000.aips-bus/2184200.usb/ci_hdrc.1/gadget/lun0/file
echo "$usb_dev_path" >  /sys/devices/soc0/soc.1/2100000.aips-bus/2184200.usb/ci_hdrc.1/gadget/lun0/file
echo 1d6b > /sys/class/android_usb_accessory/android0/idVendor
echo a4a5 > /sys/class/android_usb_accessory/android0/idProduct
killall ARMadb-driver
echo 1 > /sys/class/android_usb_accessory/f_mass_storage/inquiry_string
#sleep 1
echo mass_storage > /sys/class/android_usb_accessory/android0/functions
echo 1 > /sys/class/android_usb_accessory/android0/enable
