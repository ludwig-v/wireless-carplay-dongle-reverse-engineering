########################
# Copyright(c) 2014-2017 DongGuan HeWei Communication Technologies Co. Ltd.
# file    update_box.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    03Jun17
########################
#!/bin/sh
if [ $# -eq 1 ]; then
	imgfile=$1
elif [ -e /etc/box_product_type ]; then
	imgfile=`cat /etc/box_product_type`_Update.img
else
	imgfile=Auto_Box_Update.img
fi

export updatepath=/tmp/update
if [ -e /tmp/$imgfile ] ; then
	echo "Update OTA Start!!" > /dev/console
	echo 5 > /tmp/update_status
	sleep 2
	cd /tmp
	rm -f ./update.tar.gz
	ARMimg_maker ./$imgfile
	rm -f ./$imgfile
	echo 3 > /proc/sys/vm/drop_caches
	rm -rf $updatepath
	mkdir -p $updatepath
	tar -zxvf ./update.tar.gz -C $updatepath
	if [ $? -eq 0 ] ; then
		if [ -e $updatepath/tmp/once.sh ] ; then
			$updatepath/tmp/once.sh || (echo 7 > /tmp/update_status;exit 1)
		fi
		if [ -e /script/quickly_update.sh ]; then
			/script/quickly_update.sh && sync && echo 6 >  /tmp/update_status
		else
			cp -r $updatepath/* / && sync && echo 6 >  /tmp/update_status
		fi
		if [ $? -ne 0 ] ; then
			echo 7 > /tmp/update_status
		else
			if [ -e $updatepath/tmp/finish.sh ] ; then
				$updatepath/tmp/finish.sh
			fi
			sleep 3 
			rm -f /tmp/update_status
			sleep 1 
			sync
			reboot
		fi
	else
		echo 7 >  /tmp/update_status
	fi
fi
