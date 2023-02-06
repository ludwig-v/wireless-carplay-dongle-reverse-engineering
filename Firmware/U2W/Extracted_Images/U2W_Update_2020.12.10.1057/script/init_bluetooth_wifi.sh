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

if [ "$sdioCardID" == "0xb822" ]; then
	tar -xvf /lib/firmware/rtlbt/rtl8822_ko.tar.gz -C /tmp
	#insmod Wi-Fi module Driver
	test -e /tmp/88x2bs.ko && insmod /tmp/88x2bs.ko
	ifconfig wlan0 up;sleep 0.1;ifconfig wlan0 down
	sleep 0.1;set_wifi_mac
	#attach bluetooth module
	test -e /tmp/hci_rtk_uart.ko && insmod /tmp/hci_rtk_uart.ko
	if [ -e /usr/sbin/rtk_hciattach ]; then
		echo "Start load Bluetooth!!!"
		rtk_hciattach -s 115200 ttymxc2 rtk_h5 &
	fi
elif [ "$sdioCardID" == "0x4354" ]; then
	tar -xvf /lib/firmware/bcm/bcm4354_ko.tar.gz -C /tmp
	#attach bluetooth module
	test -e /tmp/hci_uart.ko && insmod /tmp/hci_uart.ko
	if [ -e /usr/sbin/brcm_patchram_plus ]; then
		echo "Start load Bluetooth!!!"
		brcm_patchram_plus --patchram /lib/firmware/bcm/bcm4350.hcd --baudrate 1500000 --use_baudrate_for_download --no2bytes --tosleep 200000 /dev/ttymxc2 --enable_hci &
	fi
	#insmod Wi-Fi module Driver
	test -e /tmp/bcmdhd.ko && insmod /tmp/bcmdhd.ko firmware_path=/lib/firmware/bcm/fw_bcm4354a1_ag_apsta.bin nvram_path=/lib/firmware/bcm/nvram_bcm4354_m.txt
	sleep 1;set_wifi_mac
fi

