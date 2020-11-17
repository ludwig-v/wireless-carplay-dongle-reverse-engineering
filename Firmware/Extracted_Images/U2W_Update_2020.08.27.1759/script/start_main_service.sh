########################
# Copyright(c) 2014-2015 DongGuan HeWei Communication Technologies Co. Ltd.
# file    start_main_service.sh 
# brief   
# author  Shi Kai
# version 1.0.0
# date    12Jul15
########################
#!/bin/bash
#set date
date -s 2020-01-02

#init gpio
/script/init_gpio.sh &

#copy bin and lib 
if [ -e script/copy_to_tmp.sh ]; then
	/script/copy_to_tmp.sh
fi

if [ -e /usr/sbin/fakeiOSDevice ]; then
	cp /usr/sbin/fakeiOSDevice /tmp/
	cd /tmp
	if [ -e /etc/log_com ]; then
		./fakeiOSDevice 2>&1 | tee -a /tmp/userspace.log &
	elif [ -e /etc/log_file ]; then
		./fakeiOSDevice >> /tmp/userspace.log 2>&1 &
	else
		./fakeiOSDevice >> /dev/null 2>&1 &
	fi
elif [ -e /usr/sbin/ARMadb-driver ]; then
	if [ -e /etc/log_com ]; then
		ARMadb-driver 2>&1 | tee -a /tmp/userspace.log &
	elif [ -e /etc/log_file ]; then
		ARMadb-driver >> /tmp/userspace.log 2>&1 &
	else
		ARMadb-driver >> /dev/null 2>&1 &
	fi
fi

if [ -e /usr/sbin/rtk_hciattach ]; then
	echo "Start load Bluetooth!!!"
	rtk_hciattach -s 115200 ttymxc2 rtk_h5 &
fi

sleep 2

echo "Start Carplay mdnsd!!!"
mdnsd

echo "Start Carplay IAP2&NCM driver!!!"
/script/start_iap2_ncm.sh

echo "Start NCM network"
/script/start_ncm.sh

if [ -e /usr/sbin/boa ]; then 
	echo "Web Server Service!!!"
	mkdir -p /tmp/cgi-bin/ && cp /etc/boa/cgi-bin/* /tmp/cgi-bin/
	boa
fi

#auto start wifi after some time for ota upgrade
if [ -e /usr/sbin/fakeiOSDevice ]; then
	sleep 10
	ifconfig | grep usb0 || /script/start_bluetooth_wifi.sh
fi

#check reboot
#while true
#do
#	sleep 7
#	ps | grep -v grep | grep fakeiOSDevice >> /dev/null || reboot
#done
