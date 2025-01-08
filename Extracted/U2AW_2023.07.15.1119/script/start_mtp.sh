########################
# Copyright(c) 2014-2015 DongGuan HeWei Communication Technologies Co. Ltd.
# brief   
# author  Li Jian
# version 1.0.0
# date    2020Dec21
########################
#!/bin/bash
mode=mtp,adb
setMode=`riddleBoxCfg -g USBConnectedMode`
case $setMode in
	0)
		mode=mtp,adb
		;;
	1)
		mode=mtp
		;;
	2)
		mode=adb
		;;
esac
mtp-server&
echo 0 > /sys/class/android_usb_accessory/android0/enable
test -e /etc/device_serial && head -c 16 /etc/device_serial > /sys/class/android_usb_accessory/android0/iSerial
echo 12d1 > /sys/class/android_usb_accessory/android0/idVendor
echo 0001 > /sys/class/android_usb_accessory/android0/idProduct
echo $mode > /sys/class/android_usb_accessory/android0/functions
echo 1 > /sys/class/android_usb_accessory/android0/enable
