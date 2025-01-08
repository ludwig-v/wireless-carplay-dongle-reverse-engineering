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
elif [ -e /usr/sbin/fakeAADevice ]; then
	echo "Don't start accessory driver when use fakeAADevice!!!" >> /dev/ttymxc0
	exit 0
elif [ -e /tmp/UDiskPassThroughMode ]; then
	echo "Don't start accessory driver when UDiskPassThroughMode!!!" >> /dev/ttymxc0
	exit 0
fi

if [ -e /tmp/change_udisk_mode ]; then
	riddleBoxCfg -g UdiskMode |grep 1 && test -e /script/start_accessory_mass_storage.sh && /script/start_accessory_mass_storage.sh && exit 0
else
	riddleBoxCfg -g UdiskMode |grep 1 && touch /tmp/change_udisk_mode
fi

echo 0 > /sys/class/android_usb_accessory/android0/enable
sleep 2
#echo "Magic Communication Tec." > /sys/class/android_usb_accessory/android0/iManufacturer
#echo "Auto Box" > /sys/class/android_usb_accessory/android0/iProduct
echo 1314 > /sys/class/android_usb_accessory/android0/idVendor
echo 1520 > /sys/class/android_usb_accessory/android0/idProduct
test -e /etc/usb_vendor_id && cat /etc/usb_vendor_id > /sys/class/android_usb_accessory/android0/idVendor
test -e /etc/usb_product_id && cat /etc/usb_product_id > /sys/class/android_usb_accessory/android0/idProduct
echo accessory > /sys/class/android_usb_accessory/android0/functions
echo 1 > /sys/class/android_usb_accessory/android0/enable
