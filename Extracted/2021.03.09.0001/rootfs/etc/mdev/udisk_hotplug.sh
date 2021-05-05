########################
# Copyright(c) 2014-2017 DongGuan HeWei Communication Technologies Co. Ltd.
# file    udisk_insert.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    03Jun17
########################
#!/bin/sh
echo "/dev/$MDEV $ACTION" >> /dev/console
case $ACTION in
	add)
		if [ ! -d /sys/block/*/$MDEV[0-9] ] ; then
			echo "no sda[0-9]" >> /dev/console
			mkdir -p /mnt/UPAN
			mount /dev/$MDEV /mnt/UPAN -t vfat -o rw,umask=0000,utf8=1
			blkid /dev/$MDEV | awk '{print $3}' > /tmp/udisk_uuid
			/script/update_box.sh /dev/$MDEV >> /dev/console &
		fi
		;;
	remove)
		umount -l /mnt/UPAN
		if [ `cat /tmp/update_status` == "2" ]; then 
			echo "Update Success, reboot now!!!" >> /dev/console
			reboot
		elif [ `cat /sys/class/android_usb_accessory/android0/functions` == "mass_storage" ]; then 
			echo "exit mass storage mode!!!" >> /dev/console
			killall ARMadb-driver
			/script/start_accessory.sh
			ARMadb-driver &
		fi
		rm -f /tmp/update_status
		;;
esac



