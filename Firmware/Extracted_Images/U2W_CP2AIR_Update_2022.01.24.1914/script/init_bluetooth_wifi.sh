########################
# Copyright(c) 2014-2020 DongGuan HeWei Communication Technologies Co. Ltd.
# file    init_bluetooth_wifi.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    28 920
########################
#!/bin/bash

sdioCardID=`cat /sys/bus/sdio/devices/mmc0:0001:1/device`
echo "BT WIFI Card Device ID: $sdioCardID"

#mv all wifibt rootfs tar file to tmp/
test -e /bcm4354_rootfs.tar.gz &&  mv /bcm4354_rootfs.tar.gz /tmp/
test -e /rtl8822_rootfs.tar.gz &&  mv /rtl8822_rootfs.tar.gz /tmp/
test -e /sd8987_rootfs.tar.gz &&  mv /sd8987_rootfs.tar.gz /tmp/
test -e /rtl8822cs_rootfs.tar.gz &&  mv /rtl8822cs_rootfs.tar.gz /tmp/
if [ "$sdioCardID" == "0xb822" ] || [ "$sdioCardID" == "0xc822" ]; then
	if [ "$sdioCardID" == "0xc822" ];then
		test -e /tmp/rtl8822cs_rootfs.tar.gz && mv /tmp/rtl8822cs_rootfs.tar.gz /tmp/rtl8822_rootfs.tar.gz
	fi
	if [ -e /tmp/rtl8822_rootfs.tar.gz ]; then
		tar -xvf /tmp/rtl8822_rootfs.tar.gz -C /
	fi

	tar -xvf /lib/firmware/rtlbt/rtl8822_ko.tar.gz -C /tmp
	#insmod Wi-Fi module Driver
	test -e /tmp/88x2bs.ko && insmod /tmp/88x2bs.ko
	test -e /tmp/88x2cs.ko && insmod /tmp/88x2cs.ko
	ifconfig wlan0 up;sleep 0.1;ifconfig wlan0 down
	sleep 0.1;set_wifi_mac
elif [ "$sdioCardID" == "0x4354" ]; then
	bcmBTFirmware=bcm4350.hcd
	bcmWiFiFirmware=fw_bcm4354a1_ag_apsta.bin
	bcmWiFiConfig=nvram_bcm4354_m.txt
	if [ -e /tmp/bcm4354_rootfs.tar.gz ]; then
		tar -xvf /tmp/bcm4354_rootfs.tar.gz -C /
	fi
	tar -xvf /lib/firmware/bcm/bcm4354_ko.tar.gz -C /tmp
	#insmod Wi-Fi module Driver
	test -e /tmp/bcmdhd.ko && insmod /tmp/bcmdhd.ko firmware_path=/lib/firmware/bcm/$bcmWiFiFirmware nvram_path=/lib/firmware/bcm/$bcmWiFiConfig
	ifconfig wlan0 up;sleep 0.1;set_wifi_mac
	/script/start_bluetooth_wifi.sh
elif [ "$sdioCardID" == "0x9149" ] || [ "$sdioCardID" == "0x9141" ]; then
	if [ -e /tmp/sd8987_rootfs.tar.gz ]; then
		tar -xvf /tmp/sd8987_rootfs.tar.gz -C /
	fi
	tar -xvf /lib/firmware/nxp/sd8987_ko.tar.gz -C /tmp
	#insmod Wi-Fi module Driver
	test -e /tmp/mlan.ko && insmod /tmp/mlan.ko
	test -e /tmp/moal.ko && insmod /tmp/moal.ko mod_para=nxp/wifi_mod_para.conf
	#test -e /tmp/moal.ko && insmod /tmp/moal.ko fw_name=nxp/sdiouart8987_combo_v0.bin drv_mode=2 cal_data_cfg=none uap_name=wlan cfg80211_wext=0xf ps_mode=2 auto_ds=2
	#set_wifi_mac
	/script/start_bluetooth_wifi.sh
	#tx power is low when first start AP
	/script/close_bluetooth_wifi.sh
fi
test -e /sys/bus/sdio/devices/mmc0:0001:1/device && /script/attach_bluetooth.sh &

sync
#Power LED ON
echo 2 > /sys/class/gpio/export;
echo out > /sys/class/gpio/gpio2/direction;
echo 0 >/sys/class/gpio/gpio2/value;
