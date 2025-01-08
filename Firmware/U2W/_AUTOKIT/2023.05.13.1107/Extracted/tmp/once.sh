########################
# Copyright(c) 2014-2020 DongGuan HeWei Communication Technologies Co. Ltd.
# file    once.sh
# brief   
# author  Haiguang Yin
# version 1.0.0
# date    14Apr20
########################
#!/bin/bash

setUpdateProgress() {
	echo $1 > /tmp/update_progress
}

rm /usr/lib/libjpeg*
rm /usr/lib/libturbojpeg*
rm -rf /etc/assets && ln -s /tmp /etc/assets
rm -f /usr/lib/libAirPlay*
rm -f /usr/lib/libcarlifevehicle.so*
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

rm -f /var/log/box_last_reboot.log
rm -f /data/update/*

setUpdateProgress 6
# for update new hicar sdk
#cat /etc/software_version |grep "2022.11" && rm -f /etc/hichain/extention_component.dat /etc/hichain/external_component.dat /etc/hichain/deviceInfo.txt
# for update new hicar model id
versionNum=`cat /etc/software_version |sed s/'\.'//g`
if [ $versionNum -lt "202304251700" ] && [ -e /usr/sbin/ARMHiCar ]; then
	cat /etc/box_product_type |grep "U2HW_PUBLIC" || (rm -f /etc/hichain/* /etc/bluetooth/mybluetooth*;sync)
fi
/script/phone_link_deamon.sh CarPlay stop

#use new version boa html file
if [ -e /tmp/update/etc/boa/www ]; then
	rm -rf /etc/boa/www/*
	wwwPkg=/tmp/update/etc/boa/www/www.tar.gz
	test -e $wwwPkg && (tar -xvf $wwwPkg -C /etc/boa/www/;rm -f $wwwPkg)
	cp -r /tmp/update/etc/boa/www/* /etc/boa/www/
	sync
fi
setUpdateProgress 7

sdioCardID=`cat /sys/bus/sdio/devices/mmc0:0001:1/device`
if [ "$sdioCardID" == "0xb822" ]; then
	if [ -e /tmp/update/rtl8822_rootfs.tar.gz ]; then
		tar -xvf /tmp/update/rtl8822_rootfs.tar.gz -C /tmp/update/
	fi
elif [ "$sdioCardID" == "0x4354" ] || [ "$sdioCardID" == "0x4335" ]; then
	if [ -e /tmp/update/bcm4354_rootfs.tar.gz ]; then
		tar -xvf /tmp/update/bcm4354_rootfs.tar.gz -C /tmp/update/
		if [ "$sdioCardID" == "0x4335" ]; then 
			mv /tmp/update/lib/firmware/bcm/4335/* /tmp/update/lib/firmware/bcm/
		fi
		rm -rf /tmp/update/lib/firmware/bcm/4335
	fi
elif [ "$sdioCardID" == "0x9149" ] || [ "$sdioCardID" == "0x9141" ]; then
	if [ -e /tmp/update/sd8987_rootfs.tar.gz ]; then
		tar -xvf /tmp/update/sd8987_rootfs.tar.gz -C /tmp/update/
	fi
elif [ "$sdioCardID" == "0xc822" ]; then
	if [ -e /tmp/update/rtl8822cs_rootfs.tar.gz ]; then
		tar -xvf /tmp/update/rtl8822cs_rootfs.tar.gz -C /tmp/update/
	fi
elif [ "$sdioCardID" == "0x4358" ] || [ "$sdioCardID" == "0xaa31" ]; then
	if [ -e /tmp/update/bcm4358_rootfs.tar.gz ]; then
		tar -xvf /tmp/update/bcm4358_rootfs.tar.gz -C /tmp/update/
	fi
fi
setUpdateProgress 8
rm -f /tmp/update/rtl8822_rootfs.tar.gz
rm -f /tmp/update/bcm4354_rootfs.tar.gz
rm -f /tmp/update/sd8987_rootfs.tar.gz
rm -f /tmp/update/rtl8822cs_rootfs.tar.gz
rm -f /tmp/update/bcm4358_rootfs.tar.gz
test -e /etc/wifi_name && sed -i "s/^ssid=.*/ssid=`cat /etc/wifi_name`/" /tmp/update/etc/hostapd.conf
setUpdateProgress 9

echo "Start upg"
rm /tmp/*.img;rm /tmp/update.tar.gz
mv /tmp/update/tmp/upg /tmp/;mv /tmp/update/tmp/*.img /tmp/
chmod +x /tmp/upg
/tmp/upg >> /dev/console 2>&1 || exit 1
echo "End upg"
