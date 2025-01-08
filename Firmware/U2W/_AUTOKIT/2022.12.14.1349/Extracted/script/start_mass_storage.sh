########################
# Copyright(c) 2014-2015 DongGuan HeWei Communication Technologies Co. Ltd.
# file    mount-SK.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    12Jul15
########################
#!/bin/bash
test -e /tmp/g_android_accessory.ko || exit 0
riddleBoxCfg -g UDiskPassThrough |grep 0 && echo "Udisk pass through mode closed" >> /dev/console && exit 0
#For Led Test
test -e /mnt/UPAN/color_led.txt && cp /mnt/UPAN/color_led.txt /etc/riddle.conf && exit 0

touch /tmp/UDiskPassThroughMode
if [ -e /usr/sbin/fakeiOSDevice ];then
	killall -9 fakeiOSDevice
	rmmod g_iphone && rmmod f_ptp
	echo 0 > /sys/bus/platform/devices/ci_hdrc.1/inputs/b_bus_req
	sleep 1
	insmod /tmp/storage_common.ko; insmod /tmp/g_android_accessory.ko
elif [ -e /usr/sbin/fakeCarLifeDevice ];then
	killall -9 fakeCarLifeDevice
elif [ -e /usr/sbin/fakeAADevice ];then
	killall -9 fakeAADevice
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
