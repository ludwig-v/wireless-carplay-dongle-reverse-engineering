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
test -e /sd8997_rootfs.tar.gz &&  mv /sd8997_rootfs.tar.gz /tmp/
if [ "$sdioCardID" == "0xb822" ]; then
	if [ -e /tmp/rtl8822_rootfs.tar.gz ]; then
		tar -xvf /tmp/rtl8822_rootfs.tar.gz -C /
	fi

	tar -xvf /lib/firmware/rtlbt/rtl8822_ko.tar.gz -C /tmp
	#attach bluetooth module
	test -e /tmp/rtk_hci_uart.ko && insmod /tmp/rtk_hci_uart.ko
	if [ -e /usr/sbin/rtk_hciattach ]; then
		echo "Start load Bluetooth!!!"
		rtk_hciattach -s 115200 ttymxc2 rtk_h5 &
	fi
	#insmod Wi-Fi module Driver
	test -e /tmp/88x2bs.ko && insmod /tmp/88x2bs.ko
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
	#attach bluetooth module
	test -e /tmp/hci_uart.ko && insmod /tmp/hci_uart.ko
	if [ -e /usr/sbin/brcm_patchram_plus ]; then
		echo "Start load Bluetooth!!!"
		bcmBTMac=`set_wifi_mac |sed "s/Setting Wi-Fi MAC address: 00:E0:4C/38:BA:B0/"`
		brcm_patchram_plus --patchram /lib/firmware/bcm/$bcmBTFirmware --baudrate 1000000 --use_baudrate_for_download --no2bytes --tosleep 200000 --bd_addr $bcmBTMac /dev/ttymxc2 --enable_hci &
	fi
elif [ "$sdioCardID" == "0x9149" ] || [ "$sdioCardID" == "0x9141" ]; then
	if [ -e /tmp/sd8997_rootfs.tar.gz ]; then
		tar -xvf /tmp/sd8997_rootfs.tar.gz -C /
	fi
	tar -xvf /lib/firmware/nxp/sd8997_ko.tar.gz -C /tmp
	#insmod Wi-Fi module Driver
	test -e /tmp/mlan.ko && insmod /tmp/mlan.ko
	test -e /tmp/moal.ko && insmod /tmp/moal.ko mod_para=nxp/wifi_mod_para.conf
	set_wifi_mac
	#attach bluetooth module
	test -e /tmp/hci_uart.ko && insmod /tmp/hci_uart.ko
	if [ -e /usr/sbin/hciattach ]; then
		echo "Start load Bluetooth!!!"
		hciattach /dev/ttymxc2 any 3000000 flow &
	fi
fi

sync
#Power LED ON
echo 2 > /sys/class/gpio/export;
echo out > /sys/class/gpio/gpio2/direction;
echo 0 >/sys/class/gpio/gpio2/value;
