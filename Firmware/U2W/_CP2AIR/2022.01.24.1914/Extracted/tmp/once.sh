########################
# Copyright(c) 2014-2020 DongGuan HeWei Communication Technologies Co. Ltd.
# file    once.sh
# brief   
# author  Haiguang Yin
# version 1.0.0
# date    14Apr20
########################
#!/bin/bash

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
rm -f /usr/sbin/hciattach

rm -f /var/log/box_last_reboot.log

#use new version boa html file
if [ -e /tmp/update/etc/boa/www ]; then
	wwwPkg=/tmp/update/etc/boa/www/www.tar.gz
	rm -rf /etc/boa/www/*
	test -e $wwwPkg && (tar -xvf $wwwPkg -C /etc/boa/www/;rm -f $wwwPkg)
	cp -r /tmp/update/etc/boa/www/* /etc/boa/www/
fi

sdioCardID=`cat /sys/bus/sdio/devices/mmc0:0001:1/device`
if [ "$sdioCardID" == "0xb822" ]; then
	if [ -e /tmp/update/rtl8822_rootfs.tar.gz ]; then
		tar -xvf /tmp/update/rtl8822_rootfs.tar.gz -C /tmp/update/
	fi
elif [ "$sdioCardID" == "0x4354" ]; then
	if [ -e /tmp/update/bcm4354_rootfs.tar.gz ]; then
		tar -xvf /tmp/update/bcm4354_rootfs.tar.gz -C /tmp/update/
	fi
elif [ "$sdioCardID" == "0x9149" ] || [ "$sdioCardID" == "0x9141" ]; then
	if [ -e /tmp/update/sd8987_rootfs.tar.gz ]; then
		rm -rf /lib/firmware/nxp
		tar -xvf /tmp/update/sd8987_rootfs.tar.gz -C /tmp/update/
	fi
elif [ "$sdioCardID" == "0xc822" ]; then
	if [ -e /tmp/update/rtl8822cs_rootfs.tar.gz ]; then
		tar -xvf /tmp/update/rtl8822cs_rootfs.tar.gz -C /tmp/update/
	fi
fi
rm -f /tmp/update/rtl8822_rootfs.tar.gz
rm -f /tmp/update/bcm4354_rootfs.tar.gz
rm -f /tmp/update/sd8987_rootfs.tar.gz
rm -f /tmp/update/rtl8822cs_rootfs.tar.gz

echo "Start upg"
rm /tmp/*.img;rm /tmp/update.tar.gz;mv /tmp/update/tmp/* /tmp/;chmod +x /tmp/upg;/tmp/upg
echo "End upg"
