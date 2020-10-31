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


rm -f /usr/lib/libAirPlay*
rm -f /usr/lib/libcarlifevehicle.so*
rm -f /usr/lib/libprotobuf.so*
rm -f /usr/lib/libScreenStream.so*
rm -f /usr/lib/libAudioStream.so*
rm -f /usr/lib/libmydecodejpeg.so*
rm -f /usr/lib/libmydecodejpeg.so*

rm -f /usr/lib/libimobiledevice.so*
rm -f /usr/lib/libplist*
rm -f /usr/lib/libtasn1.so*
rm -f /usr/lib/libusbmuxd.so*
rm -f /usr/lib/libavutil.so*
rm -f /usr/lib/libavcodec.so*
rm -f /usr/lib/libavformat.so*
rm -f /usr/lib/libswresample.so*

rm -f /usr/sbin/usbmuxd
#rm -f /usr/sbin/AppleCarPlay
#rm -f /usr/sbin/ARMiPhoneIAP2
rm -f /usr/sbin/ARMAndroidAuto
rm -f /usr/sbin/carlife
rm -f /usr/sbin/adb

rm -f /usr/sbin/rc.dll
rm -f /usr/sbin/rcvec.dll
rm -f /usr/sbin/ui.script
rm -f /usr/sbin/NotoSansHans-Light.otf
rm -rf /etc/bluetooth/paird_list
rm -rf /var/lib/bluetooth/*

rm -f /usr/sbin/sdptool
rm -f /usr/sbin/hciattach
#rm -f /usr/sbin/hcid
#rm -f /usr/sbin/hciconfig
#rm -f /usr/sbin/hostapd
#rm -f /usr/sbin/tinycap
#rm -f /usr/sbin/tinymix
#rm -f /usr/sbin/dtmf_decode
#rm -f /usr/sbin/hostapd
#killall mdnsd

#rm -f /etc/bluetooth/mybluetooth.tar.gz
#rm -f /etc/bluetooth_name
#rm -f /etc/wifi_name

rm -f /etc/u2w_demo
echo -n "ALL_CAR" > /etc/support_car_type
cp /tmp/update/script/check_update.sh /script

rm -f /var/log/*.log

grep "name = CarPlay" /etc/airplay.conf  || sed -i "s/^name.*/name = CarPlay/" /etc/airplay.conf
killall AppleCarPlay
killall ARMiPhoneIAP2
#killall -9 bluetoothDaemon

exit 0

cp /tmp/update/tmp/* /tmp/
FILE=/tmp/u-boot-env.img
if [ -e $FILE ]; then
	flash_erase /dev/mtd0 0xC0000 1
	dd if=$FILE of=/dev/mtd0 bs=64k seek=12 >> /dev/ttymxc0
	echo "Update uboot-env Success!"
fi
FILE=/tmp/imx6ul-14x14-evk.dtb
if [ -e $FILE ]; then
	/script/update_dtb.sh >> /dev/ttymxc0
fi
FILE=/tmp/zImage
if [ -e $FILE ]; then
	/script/update_kernel.sh >> /dev/ttymxc0
fi
sync

#killall fakeiOSDevice
#killall mdnsd
#/tmp/check_finish.sh &
