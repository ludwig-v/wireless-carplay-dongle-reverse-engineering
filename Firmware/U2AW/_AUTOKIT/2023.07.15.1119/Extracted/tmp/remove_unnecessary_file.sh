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
	#rm -f /usr/sbin/hciattach
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
			rm -f $filepath/$file
		done
	fi
}

productType=`cat /etc/box_product_type | sed 's/_.*//'`
filepath=/tmp/update
echo $productType

remove_old_file
remove_log_file

#remove wpa
support_product="U2IW,U2WR,UC2D,"
file_list="/usr/sbin/wpa_supplicant,/etc/wpa_supplicant.conf"
remove_module_file $support_product $file_list
#remove hfp
support_product="U2AW,A15W,O2W,UC2AW,"
file_list="/usr/sbin/hfpd,/etc/hfpd.conf,/etc/dbus-1/system.d/hfpd.conf"
remove_module_file $support_product $file_list
#remove fakeAA
support_product="O2W,"
file_list="/usr/sbin/fakeAADevice"
remove_module_file $support_product $file_list
#remove mtp/adb
support_product="UC2W,UC2HW,UC2HC,UC2F,UC2D,UC2WD,U2AC,O2W,"
file_list="/usr/sbin/mtp-server,/usr/sbin/adbd,/usr/sbin/am,/usr/sbin/echoDelayTest"
remove_module_file $support_product $file_list
#remove DVR 
support_product="U2WR,"
file_list="/usr/sbin/DVRServer"
remove_module_file $support_product $file_list
