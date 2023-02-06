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

cat /proc/cmdline | grep ttyLogFile && ttylog=1 || ttylog=0
if [ $ttylog -eq 1 ] && [ -e /etc/log_file ]; then
	rm -f /etc/log_com
	touch /tmp/ttyLog
	ln -s /tmp/ttyLog /tmp/userspace.log
fi

if [ -e /usr/sbin/riddleBoxCfg ]; then 
	test -e /etc/riddle.conf || (echo "create riddle.conf from old config file";riddleBoxCfg --readOld)
fi
	

#Start load Bluetooth related before main programs run
/script/init_bluetooth_wifi.sh

if [ -e /usr/sbin/fakeiOSDevice ]; then
	cp /usr/sbin/fakeiOSDevice /tmp/
	cd /tmp
	if [ -e /etc/log_com ]; then
		./fakeiOSDevice 2>&1 | tee -a /tmp/userspace.log &
	elif [ -e /etc/log_file ]; then
		if [ $ttylog -eq 1 ]; then
			./fakeiOSDevice &
		else
			./fakeiOSDevice >> /tmp/userspace.log 2>&1 &
		fi
	else
		./fakeiOSDevice >> /dev/null 2>&1 &
	fi
elif [ -e /usr/sbin/fakeCarLifeDevice ]; then
	cp /usr/sbin/fakeCarLifeDevice /tmp/
	cd /tmp
	if [ -e /etc/log_com ]; then
		./fakeCarLifeDevice 2>&1 | tee -a /tmp/userspace.log &
	elif [ -e /etc/log_file ]; then
		if [ $ttylog -eq 1 ]; then
			./fakeCarLifeDevice &
		else
			./fakeCarLifeDevice >> /tmp/userspace.log 2>&1 &
		fi
	else
		./fakeCarLifeDevice >> /dev/null 2>&1 &
	fi
elif [ -e /usr/sbin/ARMadb-driver ]; then
	insmod /tmp/storage_common.ko
	insmod /tmp/g_android_accessory.ko
	insmod /tmp/cdc_ncm.ko
	if [ -e /etc/log_com ]; then
		ARMadb-driver 2>&1 | tee -a /tmp/userspace.log &
	elif [ -e /etc/log_file ]; then
		if [ $ttylog -eq 1 ]; then
			ARMadb-driver &
		else
			ARMadb-driver >> /tmp/userspace.log 2>&1 &
		fi
	else
		ARMadb-driver >> /dev/null 2>&1 &
	fi
fi


# sleep 2

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

echo "Save Log to UPAN"
test -e /script/check_udisk_log.sh && /script/check_udisk_log.sh &

#auto start wifi after some time for ota upgrade
/script/start_bluetooth_wifi.sh
#if [ -e /usr/sbin/fakeiOSDevice ]; then
#	sleep 15
#	ifconfig | grep wlan0 || /script/start_bluetooth_wifi.sh
#fi

echo 3 > /proc/sys/vm/drop_caches
hwSecret&

#check reboot
#while true
#do
#	sleep 7
#	ps | grep -v grep | grep fakeiOSDevice >> /dev/null || reboot
#done
