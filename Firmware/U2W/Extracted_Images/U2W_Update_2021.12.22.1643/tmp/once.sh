########################
# Copyright(c) 2014-2017 DongGuan HeWei Communication Technologies Co. Ltd.
# file    udisk_insert.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    03Jun17
########################
#!/bin/sh
echo "Run $0 start!!!" >> /dev/ttymxc0
rm -f /usr/lib/libjpeg*
rm -f /usr/lib/libturbojpeg*
rm -rf /etc/assets && ln -s /tmp /etc/assets
#cp /tmp/iap2.log /mnt/UPAN/


rm -f /usr/lib/libAirPlay*
rm -f /usr/lib/libprotobuf.so*
rm -f /usr/lib/libcarlifevehicle.so*
rm -f /usr/lib/libScreenStream.so*
rm -f /usr/lib/libAudioStream.so*
rm -f /usr/lib/libmydecodejpeg.so*
rm -f /usr/lib/libmydecodejpeg.so*

rm -f /usr/lib/libavutil.so*
rm -f /usr/lib/libavcodec.so*
rm -f /usr/lib/libavformat.so*
rm -f /usr/lib/libswresample.so*

rm -f /usr/sbin/carlife
rm -f /usr/sbin/adb

rm -f /usr/sbin/ARMiPhoneHNPd 

rm -f /usr/sbin/sdptool
rm -f /usr/sbin/tinycap
#rm -f /usr/sbin/tinymix
#rm -f /usr/sbin/dtmf_decode

rm -f /usr/lib/liblzo2.so*
#rm -f /usr/lib/libssl.so*
rm -f /lib/libasan.so.1.0.0
rm -f /lib/libubsan.so.0.0.0
#killall mdnsd

if [ -e /tmp/update/tmp/busybox ]; then
	file=/tmp/update/tmp/busybox
	leftSize=`df -B 1 | grep dev/root | awk '{print $4}'`
	diff $file /bin/busybox >> /dev/null
	if [ $? -ne 0 ]; then
		size=`stat -c "%s" $file`
		echo "$file size: $size, need update" >> /dev/console
		echo "leftsize: $leftSize" >> /dev/console
		if [ $size -gt $leftSize ]; then
			echo "Flash left size not enough, Can't update $file!!!" >> /dev/console
			echo 7 >  /tmp/update_status;sleep 10;reboot;exit 1
		fi
		#test cp busybox to flash ok, then do really copy
		cp $file /bin/busybox_test
		if [ $? -eq 0 ]; then
			echo "test copy $file ok!!!" >> /dev/console
			rm -f /bin/busybox_test;sync
			cp $file /bin/busybox;sync
			test -e /tmp/update/etc/software_version && cp /tmp/update/etc/software_version /etc/software_version
			echo 6 >  /tmp/update_status;sleep 10;reboot;exit 0
		else
			echo "test copy $file failed!!!" >> /dev/console
			rm -f /bin/busybox_test;sync
			echo 7 >  /tmp/update_status;sleep 10;reboot;exit 1
		fi
	fi
fi
