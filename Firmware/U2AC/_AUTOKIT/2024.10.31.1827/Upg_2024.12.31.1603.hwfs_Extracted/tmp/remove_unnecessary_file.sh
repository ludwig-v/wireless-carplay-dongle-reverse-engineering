########################
# Copyright(c) 2014-2023 DongGuan HeWei Communication Technologies Co. Ltd.
# file    remove_unnecessary_file.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    14Apr20
########################
#!/bin/bash

remove_old_file() {
	rm -f /usr/lib/libjpeg*
	rm -f /usr/lib/libturbojpeg*
	#rm -rf /etc/assets && ln -s /tmp /etc/assets
	rm -f /usr/lib/libAirPlay*
	#rm -f /usr/lib/libcarlifevehicle.so*
	rm -f /usr/lib/libprotobuf.so*
	rm -f /usr/lib/libScreenStream.so*
	rm -f /usr/lib/libAudioStream.so*
	rm -f /usr/lib/libmydecodejpeg.so*
	rm -f /usr/lib/libAudioStream.so*
	rm -f /usr/lib/libmydecodejpeg.so*
	rm -f /usr/lib/libav*
	rm -f /usr/lib/libsw*

	rm -f /usr/lib/libimobiledevice.so*
	rm -f /usr/lib/libplist*.so*
	rm -f /usr/lib/libtasn1.so*
	rm -f /usr/lib/libusbmuxd.so*

	rm -f /usr/lib/liblzo2.so*
	rm -f /usr/lib/libdmsdpcrypto.so
	rm -f /lib/libasan.so.1.0.0
	rm -f /lib/libubsan.so.0.0.0
	rm -f /etc/box_brand_name
	rm -f /etc/usb_product_id
	#rm -f /usr/sbin/hciattach
	rm -f /usr/sbin/boxImgTools
	cat /etc/box_product_type |grep UC2HCD && rm -rf /etc/car_logo
	cat /tmp/update/etc/box_product_type |grep A15X_ && rm -f /etc/BoxHelper.apk /usr/sbin/usbmuxd /usr/sbin/ARMandroid_Mirror /usr/sbin/boxImgTools.zip /tmp/update/usr/sbin/boxImgTools.zip
}

remove_log_file() {
	rm -f /var/log/*.log
	rm -f /data/update/*
}

remove_module_file() {
	support_product_list=$1
	remove_file_list=`echo $2|tr ',' ' '`
	needRemove=0
	echo $support_product_list |grep ${productType}, || needRemove=1
	if [ $needRemove -eq 1 ]; then
		for file in $remove_file_list
		do
			echo remove $filepath/$file
			rm -rf $filepath/$file
		done
	fi
}

if [ $# -ge 2 ];then
	filepath=$1
	productType=$2
else
	productType=`cat /etc/box_product_type | sed 's/_.*//'`
	filepath=/tmp/update
	remove_old_file
	remove_log_file
fi
echo $productType


#remove wpa
support_product="U2IW,U2WR,UC2D,A15X,"
file_list="/usr/sbin/wpa_supplicant,/usr/sbin/wpa_cli,/usr/sbin/iw,/etc/wpa_supplicant.conf,/etc/p2p_supplicant.conf"
remove_module_file $support_product $file_list
#remove hfp
support_product="U2AW,A15W,O2W,UC2AW,U2AC,UC2CA,UC2AWD,UC2CAD,U2OW,"
file_list="/usr/sbin/hfpd,/etc/hfpd.conf,/etc/dbus-1/system.d/hfpd.conf"
remove_module_file $support_product $file_list
#remove fakeAA
support_product="O2W,"
file_list="/usr/sbin/fakeAADevice"
remove_module_file $support_product $file_list
#remove mtp/adb
support_product="UC2W,UC2HW,UC2HC,UC2AW,UC2F,UC2D,UC2WD,UC2HWD,UC2HCD,U2AC,O2W,UC2CA,UC2AWD,UC2CAD,"
file_list="/usr/sbin/mtp-server,/usr/sbin/adbd,/usr/sbin/am"
remove_module_file $support_product $file_list
#remove echoDelayTest/car_logo
support_product="UC2W,UC2HW,UC2HC,UC2WD,UC2HWD,UC2CA,UC2CAD,"
file_list="/usr/sbin/echoDelayTest,/etc/DTMF-14809414327.pcm,/etc/car_logo"
remove_module_file $support_product $file_list
#remove hichain
support_product="A15HW,A15W,U2HW,U2HP,U2HC,UC2HW,UC2HC,UC2HCD,UC2HWD,"
file_list="/etc/hichain"
remove_module_file $support_product $file_list
#remove iccoa
support_product="U2IW,"
file_list="/etc/iccoa"
remove_module_file $support_product $file_list
#remove DVR 
support_product="U2WR,"
file_list="/usr/sbin/DVRServer"
remove_module_file $support_product $file_list
