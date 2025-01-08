########################
# Copyright(c) 2014-2015 DongGuan HeWei Communication Technologies Co. Ltd.
# file    start_main_service.sh 
# brief   
# author  Shi Kai
# version 1.0.0
# date    12Jul15
########################
#!/bin/bash

runMainProcess() {
	processName=$1
	cp /usr/sbin/$processName /tmp/
	cd /tmp
	if [ -e /etc/log_com ]; then
		./$processName 2>&1 | tee -a /tmp/userspace.log &
	elif [ -e /etc/log_file ]; then
		if [ $ttylog -eq 1 ]; then
			./$processName &
		else
			./$processName >> /tmp/userspace.log 2>&1 &
		fi
	else
		./$processName >> /dev/null 2>&1 &
	fi
}

#use /dev/urandom to avoid block call in openssl
rm -f /dev/random; ln -s /dev/urandom /dev/random

#open kernel log timestamp
echo y > /sys/module/printk/parameters/time
echo 4  > /proc/sys/kernel/printk
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
	test -e /etc/riddle.conf || (echo "create riddle.conf from default config file";cp /etc/riddle_default.conf /etc/riddle.conf)
	test -e /etc/riddle.conf || (echo "create riddle.conf from old config file";riddleBoxCfg --readOld)
fi
	

#Start load Bluetooth related before main programs run
/script/init_bluetooth_wifi.sh

if [ -e /usr/sbin/fakeiOSDevice ]; then
	runMainProcess fakeiOSDevice
elif [ -e /usr/sbin/fakeCarLifeDevice ]; then
	test -e /usr/sbin/ARMHiCar && insmod /tmp/cdc_ncm.ko
	runMainProcess fakeCarLifeDevice
elif [ -e /usr/sbin/fakeAADevice ]; then
	runMainProcess fakeAADevice
elif [ -e /usr/sbin/ARMadb-driver ]; then
	sleep `riddleBoxCfg -g BoxConfig_DelayStart`
	insmod /tmp/storage_common.ko
	insmod /tmp/g_android_accessory.ko
	test -e /etc/usb_product_id && /script/start_accessory.sh &
	insmod /tmp/cdc_ncm.ko
	runMainProcess ARMadb-driver
fi


# sleep 2

echo "Start Carplay mdnsd!!!"
mdnsd

echo "Start Carplay IAP2&NCM driver!!!"
/script/start_iap2_ncm.sh

echo "Start NCM network"
/script/start_ncm.sh

# delay start other service, make main service start quickly
sleep 5

if [ -e /usr/sbin/boa ]; then 
	echo "Web Server Service!!!"
	#remove old version boa web, now new verson boa web support active 
	test -e /etc/boa_old && rm -rf /etc/boa_old
	mkdir -p /tmp/cgi-bin/ && cp /etc/boa/cgi-bin/* /tmp/cgi-bin/
	test -e /etc/uuid && rm -rf /etc/boa_old
	if [ -e /etc/boa_old ]; then
		boa -f /etc/boa_old/boa.conf
	else
		test -e /etc/boa/www/www.tar.gz && (tar -xvf /etc/boa/www/www.tar.gz -C /etc/boa/www/;rm -f /etc/boa/www/www.tar.gz)
		boa
	fi
fi

echo "Save Log to UPAN"
test -e /script/check_udisk_log.sh && /script/check_udisk_log.sh &

#auto start wifi after some time for ota upgrade
if [ -e /usr/sbin/boa ]; then
	/script/start_bluetooth_wifi.sh
fi

echo 3 > /proc/sys/vm/drop_caches
hwSecret&

/script/cpu_UsageRate.sh &
#check reboot
#while true
#do
#	sleep 7
#	ps | grep -v grep | grep fakeiOSDevice >> /dev/null || reboot
#done
