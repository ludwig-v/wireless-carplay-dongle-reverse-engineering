########################
# Copyright(c) 2014-2015 DongGuan HeWei Communication Technologies Co. Ltd.
# file    mount-SK.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    12Jul15
########################
#!/bin/bash
if [ -e /usr/sbin/fakeiOSDevice ]; then
	echo "Don't start accessory driver when use fakeiOSDevice!!!" >> /dev/ttymxc0
	exit 0
elif [ -e /usr/sbin/fakeCarLifeDevice ]; then
	echo "Don't start accessory driver when use fakeCarLifeDevice!!!" >> /dev/ttymxc0
	exit 0
fi

echo 0 > /sys/class/android_usb_accessory/android0/enable
sleep 1
#echo "Magic Communication Tec." > /sys/class/android_usb_accessory/android0/iManufacturer
#echo "Auto Box" > /sys/class/android_usb_accessory/android0/iProduct
echo 1314 > /sys/class/android_usb_accessory/android0/idVendor
echo 1520 > /sys/class/android_usb_accessory/android0/idProduct
echo accessory > /sys/class/android_usb_accessory/android0/functions
echo 1 > /sys/class/android_usb_accessory/android0/enable
