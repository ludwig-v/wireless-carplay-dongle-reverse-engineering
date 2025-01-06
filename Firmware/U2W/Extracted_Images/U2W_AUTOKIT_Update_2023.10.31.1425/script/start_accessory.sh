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

iManufacturer=`riddleBoxCfg -g USBManufacturer`
iProduct=`riddleBoxCfg -g USBProduct`
idVendor=`riddleBoxCfg -g USBVID`
idProduct=`riddleBoxCfg -g USBPID`
iSerial=`riddleBoxCfg -g USBSerial`
uuid=`riddleBoxCfg --uuid`

echo 0 > /sys/class/android_usb_accessory/android0/enable
sleep 2
#echo "Magic Communication Tec." > /sys/class/android_usb_accessory/android0/iManufacturer
#echo "Auto Box" > /sys/class/android_usb_accessory/android0/iProduct

if [ -n "$iManufacturer" ]; then
	echo -n $iManufacturer > /sys/class/android_usb_accessory/android0/iManufacturer	
fi
if [ -n "$iProduct" ]; then
	echo -n $iProduct > /sys/class/android_usb_accessory/android0/iProduct
fi
if [ -n "$idVendor" ]; then
	echo -n $idVendor > /sys/class/android_usb_accessory/android0/idVendor
else
	echo 1314 > /sys/class/android_usb_accessory/android0/idVendor
fi
if [ -n "$idProduct" ]; then
	echo -n $idProduct > /sys/class/android_usb_accessory/android0/idProduct
else
	echo 1520 > /sys/class/android_usb_accessory/android0/idProduct
fi
if [ "$iSerial" == "UUID" ]; then
	echo -n $uuid > /sys/class/android_usb_accessory/android0/iSerial
elif [ -n "$iSerial" ]; then
	echo -n $iSerial > /sys/class/android_usb_accessory/android0/iSerial
fi
echo accessory > /sys/class/android_usb_accessory/android0/functions
echo 1 > /sys/class/android_usb_accessory/android0/enable
