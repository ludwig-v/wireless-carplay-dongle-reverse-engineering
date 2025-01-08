########################
# Copyright(c) 2014-2020 DongGuan HeWei Communication Technologies Co. Ltd.
# file    attach_bluetooth.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    28 920
########################
#!/bin/bash

attach_bluetooth() {
	echo "Start load Bluetooth!!!"
	sdioCardID=`cat /sys/bus/sdio/devices/mmc0:0001:1/device`
	if [ "$sdioCardID" == "0xb822" ] || [ "$sdioCardID" == "0xc822" ] || [ "$sdioCardID" == "0xb733" ]; then
		#attach bluetooth module
		test -e /tmp/rtk_hci_uart.ko && insmod /tmp/rtk_hci_uart.ko
		if [ -e /usr/sbin/rtk_hciattach ]; then
			rtk_hciattach -s 115200 ttymxc2 rtk_h5
			# improve hfp voice quality 
			hciconfig hci0 scomtu 240:32
		fi
	elif [ "$sdioCardID" == "0x4354" ] || [ "$sdioCardID" == "0x4335" ]; then
		bcmBTFirmware=bcm4350.hcd
		#attach bluetooth module
		test -e /tmp/hci_uart.ko && insmod /tmp/hci_uart.ko
		if [ -e /usr/sbin/brcm_patchram_plus ]; then
			bcmBTMac=`set_wifi_mac |sed "s/Setting Wi-Fi MAC address: 00:E0:4C/38:BA:B0/"`
			brcm_patchram_plus --patchram /lib/firmware/bcm/$bcmBTFirmware --baudrate 1000000 --use_baudrate_for_download --no2bytes --tosleep 200000 --bd_addr $bcmBTMac /dev/ttymxc2 --enable_hci &
			#wait brcm_patchram_plus finish, overtime 12 secoond
			sleep 6;hciconfig hci0 || (sleep 3;hciconfig hci0 || sleep 3)
		fi
	elif [ "$sdioCardID" == "0x4358" ] || [ "$sdioCardID" == "0xaa31" ]; then
		bcmBTFirmware=BCM4358A3.hcd
		test -e /tmp/hci_uart.ko && insmod /tmp/hci_uart.ko
		if [ -e /usr/sbin/brcm_patchram_plus ]; then
			bcmBTMac=`set_wifi_mac |sed "s/Setting Wi-Fi MAC address: 00:E0:4C/38:BA:B0/"`
			brcm_patchram_plus --patchram /lib/firmware/bcm/$bcmBTFirmware --baudrate 3000000 --use_baudrate_for_download --no2bytes --tosleep 200000 --bd_addr $bcmBTMac /dev/ttymxc2 --enable_hci &
			#wait brcm_patchram_plus finish, overtime 10 secoond
			sleep 4;hciconfig hci0 || (sleep 3;hciconfig hci0 || sleep 3)
			hciconfig hci0 up
			# improve hfp voice quality 
			hciconfig hci0 scomtu 240:32
			# route sco data to hci
			hcitool -i hci0 cmd 0x3f 0x1c 0x01 0x02 0x00 0x00 0x00
		fi
	elif [ "$sdioCardID" == "0x9149" ] || [ "$sdioCardID" == "0x9141" ]; then
		#8987需要判断WiFi加载成功才加载蓝牙，否则会卡死
		if [ -e /sys/class/net/wlan0/address ]; then
			#attach bluetooth module
			test -e /tmp/hci_uart.ko && insmod /tmp/hci_uart.ko
			if [ -e /usr/sbin/hciattach ]; then
				#hciattach /dev/ttymxc2 any 115200 flow
				#hciconfig hci0 up
				#hcitool cmd 0x3f 0x0009 0xc0 0xc6 0x2d 0x00
				#killall hciattach;sleep 2;rmmod hci_uart
				#lsmod |grep hci_uart && (sleep 2;rmmod hci_uart)
				#insmod /tmp/hci_uart.ko
				hciattach /dev/ttymxc2 any 3000000 flow
				hciconfig hci0 up
				hciconfig hci0 reset
				# improve hfp voice quality 
				hciconfig hci0 scomtu 240:32
				# route sco data to hci
				hcitool -i hci0 cmd 0x3f 0x1d 0x00
			fi
		fi
	fi
	echo "End load Bluetooth!!!"
}

reset_bluetooth() {
	echo "Reset Bluetooth!!!"
	test -e /usr/sbin/rtk_hciattach && killall rtk_hciattach
	test -e /usr/sbin/brcm_patchram_plus && killall brcm_patchram_plus
	test -e /usr/sbin/hciattach && killall hciattach
	lsmod |grep rtk_hci_uart && (sleep 2;rmmod rtk_hci_uart)
	lsmod |grep hci_uart && (sleep 2;rmmod hci_uart)
	echo 1 >/sys/class/gpio/gpio1/value;
	sleep 0.1
	echo 0 >/sys/class/gpio/gpio1/value;
}

cp /usr/sbin/bluetoothDaemon /tmp/bin/
bluetoothDaemon -n &

attach_bluetooth
hciconfig hci0
while [ $? -ne 0 ]; do
	reset_bluetooth
	attach_bluetooth
	hciconfig hci0
done
echo "Attach Bluetooth success!!!"

if [ -e /mnt/UPAN/BT_Debug ]; then
	cp /mnt/UPAN/BT_Debug/hcidump /tmp/bin/
	cp /mnt/UPAN/BT_Debug/libncurses.so.5 /tmp/lib/
	chmod +x /tmp/bin/hcidump
	hcidump -w /mnt/UPAN/BT_Debug/bt.cfa &
fi       

touch /tmp/.hciattach_done
