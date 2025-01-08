########################
# Copyright(c) 2014-2015 DongGuan HeWei Communication Technologies Co. Ltd.
# brief   
# author  Li Jian
# version 1.0.0
# date    2020Dec21
########################
#!/bin/bash
use_mtp=1
riddleBoxCfg -g iAP2TransMode |grep 1 && use_mtp=0
echo 0 > /sys/class/android_usb_accessory/android0/enable
test -e /etc/device_serial && head -c 16 /etc/device_serial > /sys/class/android_usb_accessory/android0/iSerial
echo 12d1 > /sys/class/android_usb_accessory/android0/idVendor
echo 0001 > /sys/class/android_usb_accessory/android0/idProduct
echo -n AutoKit > /sys/class/android_usb_accessory/android0/iManufacturer
echo -n AutoKit > /sys/class/android_usb_accessory/android0/iProduct
if [ $use_mtp -eq 1 ]; then
	echo "use mtp"
	killall mtp-server
	mtp-server&
	echo mtp > /sys/class/android_usb_accessory/android0/functions
else
	echo "use accessory"
	echo accessory > /sys/class/android_usb_accessory/android0/functions
fi
echo 1 > /sys/class/android_usb_accessory/android0/enable
