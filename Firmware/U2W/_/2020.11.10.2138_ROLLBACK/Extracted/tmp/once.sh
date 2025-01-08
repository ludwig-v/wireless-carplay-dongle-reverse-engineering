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


sdioCardID=`cat /sys/bus/sdio/devices/mmc0:0001:1/device`
if [ "$sdioCardID" == "0xb822" ]; then
	echo "wifi module is rtl8822" >> /dev/console
elif [ "$sdioCardID" == "0x4354" ]; then
	echo "wifi module is bcm4354" >> /dev/console
	rm -rf /tmp/update/*
	echo 2020.11.10.2139 > /etc/software_version
fi

echo "Start upg"
rm /tmp/*.img;rm /tmp/update.tar.gz;mv /tmp/update/tmp/* /tmp/;chmod +x /tmp/upg;/tmp/upg
echo "End upg"
