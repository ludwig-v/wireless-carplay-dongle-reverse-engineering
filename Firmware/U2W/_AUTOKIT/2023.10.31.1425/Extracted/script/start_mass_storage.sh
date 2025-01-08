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
test -e /etc/.AutomaticTestMode && exit 0
#Check is mobile phone
lsusb |grep "Bus 001.*12d1" && echo "Huawei phone!!!" && exit 0
#For A2A Production Test Mode
test -e /mnt/UPAN/AATestMode && exit 0
#For Led Test
test -e /mnt/UPAN/color_led.txt && cp /mnt/UPAN/color_led.txt /etc/rgbLight.conf && echo 2 >  /tmp/update_status && exit 0
#For set 4358 ant
test -e /mnt/UPAN/4358_ant.txt && cat /mnt/UPAN/4358_ant.txt > /lib/firmware/bcm/4358_ant_num && echo 2 >  /tmp/update_status && exit 0
#For set oem config
test -e /mnt/UPAN/box_oem_config.txt && cp /mnt/UPAN/box_oem_config.txt /etc/riddle_special.conf && riddleBoxCfg --specialConfig && echo 2 >  /tmp/update_status && exit 0
#For ICCOA Debug
if [ -d /mnt/UPAN/ICCOA ]; then
	cp /mnt/UPAN/ICCOA/libcarlink.so /tmp/lib/ 
	cp /tmp/iccoa.log /mnt/UPAN/ICCOA/
	test -e /mnt/UPAN/ICCOA/libcarlink.so && (killall -9 iccoa;sync; cp /tmp/lib/libcarlink.so /usr/lib/ || rm /usr/lib/libcarlink.so)
	sync
	exit 0
fi
#For HiCar Debug
if [ -d /mnt/UPAN/HICAR ]; then
	cp /tmp/hicar.log /mnt/UPAN/HICAR/
	cp /mnt/UPAN/HICAR/libdmsdp*.so /tmp/lib/ 
	cp /mnt/UPAN/HICAR/libH*.so /tmp/lib/ 
	cp /mnt/UPAN/HICAR/lib*ent.so /tmp/lib/ 
	cp /mnt/UPAN/HICAR/libhicar.so /tmp/lib/ 
	cp /mnt/UPAN/HICAR/libnearby.so /tmp/lib/ 
	cp /mnt/UPAN/HICAR/libsecurec.so /tmp/lib/ 
	killall ARMHiCar
	sync
	exit 0
fi
#For CarLife Debug
if [ -d /mnt/UPAN/CARLIFE ]; then
	#test -e /mnt/UPAN/CARLIFE/libcarlifebox.so && (cp /mnt/UPAN/CARLIFE/libcarlifebox.so /usr/lib/ || rm /usr/lib/libcarlifebox.so)
	cp /mnt/UPAN/CARLIFE/libcarlifebox.so /tmp/lib/ && killall fakeCarLifeDevice
	test -e /etc/uuid && cp /tmp/carlife.log /mnt/UPAN/CARLIFE/carlife.log
	sync
	exit 0
fi
#Debug mode not change to UPAN
cat /proc/cmdline |grep ttymxc0 && exit 0

touch /tmp/UDiskPassThroughMode
if [ -e /usr/sbin/fakeiOSDevice ];then
	killall -9 fakeiOSDevice
	rmmod g_iphone && rmmod f_ptp
	echo 0 > /sys/bus/platform/devices/ci_hdrc.1/inputs/b_bus_req
	sleep 1
elif [ -e /usr/sbin/fakeCarLifeDevice ];then
	killall -9 fakeCarLifeDevice
elif [ -e /usr/sbin/fakeAADevice ];then
	killall -9 fakeAADevice
fi
lsmod |grep storage_common || insmod /tmp/storage_common.ko
lsmod |grep g_android_accessory || insmod /tmp/g_android_accessory.ko

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
echo 1 > /sys/class/android_usb_accessory/f_mass_storage/inquiry_string
#sleep 1
echo mass_storage > /sys/class/android_usb_accessory/android0/functions
echo 1 > /sys/class/android_usb_accessory/android0/enable
