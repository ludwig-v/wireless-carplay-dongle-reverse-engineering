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
	setUpdateProgress 1
	echo 5 > /tmp/update_status
	sleep 2
	setUpdateProgress 2
	cd /tmp
	rm -f ./update.tar.gz
	ARMimg_maker ./$imgfile
	setUpdateProgress 3
	rm -f ./$imgfile
	echo 3 > /proc/sys/vm/drop_caches
	rm -rf $updatepath
	mkdir -p $updatepath
	tar -zxvf ./update.tar.gz -C $updatepath && rm -f ./update.tar.gz
	return $?
}

decompressHwfs() {
	echo "decompress $hwfsfile" >> /dev/console
	echo 5 > /tmp/update_status
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
		rm -rf $updatepath;sync;echo 3 > /proc/sys/vm/drop_caches
		echo 7 > /tmp/update_status
	else
		setUpdateProgress 100
		echo 6 >  /tmp/update_status
		sync
		sleep 4 
		reboot
	fi
}

if [ $# -eq 1 ]; then
	imgfile=$1
	#TODO set hwfsfile
elif [ -e /etc/box_product_type ]; then
	imgfile=`cat /etc/box_product_type`_Update.img
	hwfsfile=`cat /etc/box_product_type`_Update.zip
else
	imgfile=Auto_Box_Update.img
	hwfsfile=Auto_Box_Update.zip
fi

export updatepath=/tmp/update
if [ -e /tmp/$imgfile ] ; then
	echo "Update OTA Start!!" > /dev/console
	decompressImg
	if [ $? -eq 0 ] ; then
		doUpdate
	else
		echo 7 >  /tmp/update_status
	fi
elif [ -e /tmp/$hwfsfile ] ; then
	echo "Update hwfs OTA Start!!" > /dev/console
	decompressHwfs
	if [ $? -eq 0 ] ; then
		doUpdate
	else
		echo 7 >  /tmp/update_status
	fi
fi
