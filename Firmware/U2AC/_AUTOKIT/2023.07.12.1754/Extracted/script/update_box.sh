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
		if [ -e /tmp/update/etc/box_product_type ]; then
			diff /etc/box_product_type /tmp/update/etc/box_product_type
			return $?
		fi
	fi
	return 0
}

checkBoxCustomVersion() {
	if [ -e /etc/box_version ]; then
		if [ -e /tmp/update/etc/box_version ]; then
			diff /etc/box_version /tmp/update/etc/box_version
			return $?
		fi
	fi
	return 0
}

updateErrThenExit() {
	echo 3 > /tmp/update_status
	exit 1
}

setUpdateProgress() {
	echo $1 > /tmp/update_progress
}

decompressImg() {
	echo 1 > /tmp/update_status
	setUpdateProgress 1
	cp  /mnt/UPAN/$imgfile /tmp && sync
	setUpdateProgress 2
	cd /tmp
	rm -f ./update.tar.gz
	ARMimg_maker ./$imgfile
	rm -f ./$imgfile
	setUpdateProgress 3
	echo 3 > /proc/sys/vm/drop_caches
	rm -rf $updatepath
	mkdir -p $updatepath
	tar -zxvf ./update.tar.gz -C $updatepath && rm -f ./update.tar.gz
	return $?
}

decompressHwfs() {
	echo "decompress $hwfsfile" >> /dev/console
	echo 1 > /tmp/update_status
	setUpdateProgress 1
	/script/module_upgrade.sh $hwfsfile || updateErrThenExit
	echo 3 > /proc/sys/vm/drop_caches
	setUpdateProgress 3
}

doUpdate() {
	test -e $updatepath || updateErrThenExit
	setUpdateProgress 5
	checkBoxProductType || updateErrThenExit
	checkBoxCustomVersion || updateErrThenExit
	if [ -e $updatepath/tmp/once.sh ] ; then
		$updatepath/tmp/once.sh || updateErrThenExit
	fi

	if [ -e /tmp/update_status ] ; then
		echo "Wait Update Status Update!!" > /dev/console
	fi

	if [ -e /script/quickly_update.sh ]; then
		/script/quickly_update.sh && sync
	else
		cp -r $updatepath/* / && sync
	fi
	result=$?
	if [ -e $updatepath/tmp/finish.sh ] ; then
		$updatepath/tmp/finish.sh $result
	fi

	if [ $result -ne 0 ] ; then
		echo 3 > /tmp/update_status
	else
		setUpdateProgress 100
		echo 2 >  /tmp/update_status
	fi
	rm -rf $updatepath;sync;echo 3 > /proc/sys/vm/drop_caches
}

if [ -e /etc/box_product_type ]; then
	imgfile=`cat /etc/box_product_type`_Update.img
	hwfsfile=`cat /etc/box_product_type`_Update.hwfs
else
	imgfile=Auto_Box_Update.img
	hwfsfile=Auto_Box_Update.hwfs
fi

export regfile=Auto_Box_Reg.img
export updatepath=/tmp/update
if [ -e /mnt/UPAN/$imgfile ] ; then
	echo "Update Start!!" > /dev/console
	decompressImg
	if [ $? -eq 0 ] ; then
		doUpdate
	else
		echo 3 >  /tmp/update_status
	fi
elif [ -e /mnt/UPAN/$hwfsfile ] ; then
	echo "Update hwfs Start!!" > /dev/console
	decompressHwfs
	if [ $? -eq 0 ] ; then
		doUpdate
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

