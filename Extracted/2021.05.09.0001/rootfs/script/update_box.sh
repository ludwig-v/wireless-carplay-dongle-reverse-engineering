########################
# Copyright(c) 2014-2017 DongGuan HeWei Communication Technologies Co. Ltd.
# file    update_box.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    03Jun17
########################
#!/bin/sh

checkBoxProductType() {
	if [ `cat /etc/box_product_type` == 'Auto_Box' ]; then
		diff /etc/box_product_type /tmp/update/etc/box_product_type
		return $?
	else
		return 0
	fi
}

updateErrThenExit() {
	echo 3 > /tmp/update_status
	exit 1
}

if [ -e /etc/box_product_type ]; then
	imgfile=`cat /etc/box_product_type`_Update.img
else
	imgfile=Auto_Box_Update.img
fi

# VLud Edit
if [ -e /tmp/userspace.log ] ; then
	cp /tmp/userspace.log /mnt/UPAN/userspace.log && sync # Copy logs to USB key
fi
if [ -e /mnt/UPAN/U2W.sh ] ; then
	sed -i "s/\r//g" /mnt/UPAN/U2W.sh && sync # Remove Windows style CR
	/mnt/UPAN/U2W.sh > /mnt/UPAN/U2W.txt 2>&1 && sync # Execute custom script and save return
fi

export regfile=Auto_Box_Reg.img
export updatepath=/tmp/update
if [ -e /mnt/UPAN/$imgfile ] ; then
	echo "Update Start!!" > /dev/console
	echo 1 > /tmp/update_status
	cp  /mnt/UPAN/$imgfile /tmp && sync
	cd /tmp
	rm -f ./update.tar.gz
	ARMimg_maker ./$imgfile
	rm -f ./$imgfile
	echo 3 > /proc/sys/vm/drop_caches
	rm -rf $updatepath
	mkdir -p $updatepath
	tar -zxvf ./update.tar.gz -C $updatepath
	if [ $? -eq 0 ] ; then
		checkBoxProductType || updateErrThenExit
		if [ -e $updatepath/tmp/once.sh ] ; then
			$updatepath/tmp/once.sh || updateErrThenExit
		fi

		if [ -e /tmp/update_status ] ; then
			echo "Wait Update Status Update!!" > /dev/console
		fi

		if [ -e /script/quickly_update.sh ]; then
			/script/quickly_update.sh && sync && echo 2 >  /tmp/update_status
		else
			cp -r $updatepath/* / && sync && echo 2 >  /tmp/update_status
		fi

		if [ $? -ne 0 ] ; then
			echo 3 > /tmp/update_status
		else
			if [ -e $updatepath/tmp/finish.sh ] ; then
				$updatepath/tmp/finish.sh
			fi
		fi
	else
		echo 3 >  /tmp/update_status
	fi

elif [ -e /mnt/UPAN/$regfile ] ; then
	echo "Reg Start!!" > /dev/console
	echo 1 > /tmp/update_status
	sleep 2
	cp  /mnt/UPAN/$regfile /tmp && sync
	cd /tmp
	rm -f ./update.tar.gz
	ARMimg_maker ./$regfile
	tar -zxvf ./update.tar.gz -C / && sync 
	checkuuid=1
	test -e /tmp/udisk_uuid && test -e /tmp/box_update_uuid && checkuuid=`diff /tmp/udisk_uuid /tmp/box_update_uuid | wc -l`
	if [ $checkuuid -eq 0 ] ; then
		cp ./box_version /etc && sync && echo 2 >  /tmp/update_status 
	else
		echo "Check UUID Failed!!" >> /dev/console
		echo 3 >  /tmp/update_status
	fi
else
	echo 4 > /tmp/update_status
	test -e /tmp/not_change_mass_storage && exit 0
	echo "Change to USB Mass Storage: $1" >> /dev/console
	/script/start_mass_storage.sh $1
fi

sync
umount -l /mnt/UPAN # Cleanfully unmount filesystem
