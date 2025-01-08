########################
# Copyright(c) 2014-2015 DongGuan HeWei Communication Technologies Co. Ltd.
# file    start_adb_mass_storage.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    12Jul15
########################
#!/bin/bash
if [ ! -e /tmp/ram_fat32.img ];then 
	echo 0 > /sys/class/android_usb_accessory/android0/enable

	echo "Create RAM fat32 fs"
	dd if=/dev/zero of=/tmp/ram_fat32.img bs=8M count=1
	mkfs.fat -s 128 -n APK /tmp/ram_fat32.img
	losetup /dev/loop1 /tmp/ram_fat32.img
	mkdir -p /tmp/UPAN
	mount /dev/loop1 /tmp/UPAN -t vfat -o utf8=1
	cp /etc/BoxHelper.apk /tmp/UPAN/;sync
	umount /tmp/UPAN

	usb_dev_path=/dev/loop1
	echo "$usb_dev_path" >  /sys/devices/soc0/soc.0/2100000.aips-bus/2184200.usb/ci_hdrc.1/gadget/lun0/file
	echo "$usb_dev_path" >  /sys/devices/soc0/soc.1/2100000.aips-bus/2184200.usb/ci_hdrc.1/gadget/lun0/file
	echo 1 > /sys/class/android_usb_accessory/f_mass_storage/inquiry_string
	#test -e /etc/device_serial && cat /etc/device_serial > /sys/class/android_usb_accessory/android0/iSerial
	echo accessory,mass_storage > /sys/class/android_usb_accessory/android0/functions
	sleep 1
	echo 1 > /sys/class/android_usb_accessory/android0/enable
fi

exit 0
