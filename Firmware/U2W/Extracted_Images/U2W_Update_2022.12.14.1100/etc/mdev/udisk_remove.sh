########################
# Copyright(c) 2014-2017 DongGuan HeWei Communication Technologies Co. Ltd.
# file    udisk_remove.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    03Jun17
########################
#!/bin/sh
restartMainProcess() {
	cd /tmp
	if [ -e /usr/sbin/fakeiOSDevice ];then
		echo 0 > /sys/class/android_usb_accessory/android0/enable
		echo 1 > /sys/class/android_usb_accessory/f_mass_storage/uninstall
		rmmod g_android_accessory && rmmod storage_common
		./fakeiOSDevice >> /dev/console 2>&1 &
	elif [ -e /usr/sbin/fakeCarLifeDevice ];then
		./fakeCarLifeDevice >> /dev/console 2>&1 &
	elif [ -e /usr/sbin/fakeAADevice ];then
		./fakeAADevice >> /dev/console 2>&1 &
	else
		/script/start_accessory.sh
		./ARMadb-driver >> /dev/console 2>&1 &
	fi
}

echo "/dev/$MDEV PLUG OUT" >> /dev/console
umount -l /mnt/UPAN
if [ `cat /tmp/update_status` == "2" ]; then 
	echo "Update Success, reboot now!!!" >> /dev/console
	reboot
elif [ `cat /sys/class/android_usb_accessory/android0/functions` == "mass_storage" ]; then 
	echo "exit mass storage mode!!!" >> /dev/console
	rm -f /tmp/update_status /tmp/UDiskPassThroughMode
	killall ARMadb-driver
	restartMainProcess
fi

