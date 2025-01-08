########################
# Copyright(c) 2014-2017 DongGuan HeWei Communication Technologies Co. Ltd.
# file    udisk_remove.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    03Jun17
########################
#!/bin/sh
echo "/dev/$MDEV PLUG OUT" >> /dev/console
umount -l /mnt/UPAN
if [ `cat /tmp/update_status` == "2" ]; then 
	echo "Update Success, reboot now!!!" >> /dev/console
	reboot
elif [ `cat /sys/class/android_usb_accessory/android0/functions` == "mass_storage" ]; then 
	echo "exit mass storage mode!!!" >> /dev/console
	rm -f /tmp/update_status
	killall ARMadb-driver
	/script/start_accessory.sh
	ARMadb-driver &
fi

