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
	cp /usr/sbin/$processName /tmp/bin/
	
	if [ -e /tmp/UDiskPassThroughMode ]; then
		echo "Not Start Main Process in UDiskPassThroughModek!!!"
		return
	fi

	if [ -e /etc/log_com ]; then
		$processName 2>&1 | tee -a /tmp/userspace.log &
	elif [ -e /etc/log_file ]; then
		if [ $ttylog -eq 1 ]; then
			$processName &
		else
			$processName >> /tmp/userspace.log 2>&1 &
		fi
	else
		$processName >> /dev/null 2>&1 &
	fi
}

#set date
date -s "2020-01-02 `date +%T`" &
#use /dev/urandom to avoid block call in openssl
rm -f /dev/random && ln -s /dev/urandom /dev/random &

#open kernel log timestamp
echo y > /sys/module/printk/parameters/time && echo 7 > /proc/sys/kernel/printk &

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
#Check Auto Test Mode
if [ -e /etc/.AutomaticTestMode ]; then
	AutomaticTest  2>&1 | tee -a /tmp/AutoTest.log
	exit 0
fi

if [ -e /usr/sbin/ARMadb-driver ]; then
	runMainProcess ARMadb-driver
fi


sleep 2

#Satrt led daemon
test -e /usr/sbin/colorLightDaemon && colorLightDaemon &

echo "Start Carplay mdnsd!!!"
cp /usr/sbin/mdnsd /tmp/bin/
mdnsd

echo "Start Carplay IAP2&NCM driver!!!"
/script/start_iap2_ncm.sh

echo "Start NCM network"
/script/start_ncm.sh

# delay start other service, make main service start quickly
sleep 10

if [ -e /usr/sbin/boa ]; then 
	echo "Web Server Service!!!"
	cp /usr/sbin/boa /tmp/bin/
	#remove old version boa web, now new verson boa web support active 
	test -e /etc/boa_old && rm -rf /etc/boa_old
	#copy cgi and www to tmp
	test -e /etc/boa/www/www.tar.gz && (tar -xvf /etc/boa/www/www.tar.gz -C /etc/boa/www/;rm -f /etc/boa/www/www.tar.gz)
	mkdir -p /tmp/boa; cp -r /etc/boa/www /tmp/boa/; cp -r /etc/boa/cgi-bin /tmp/boa/
	boa
	#auto start wifi after some time for ota upgrade
	ps |grep udhcpd|grep -v grep || /script/start_bluetooth_wifi.sh
	#open boa web log when debug mode
	cat /proc/cmdline |grep ttymxc0 && (mkdir -p /tmp/boa/logs/ && ln -s /tmp/userspace.log /tmp/boa/log)
fi

echo "Save Log to UPAN"
test -e /script/check_udisk_log.sh && /script/check_udisk_log.sh &


echo 3 > /proc/sys/vm/drop_caches
hwSecret&

/script/cpu_UsageRate.sh &
#check log size
/script/check_log_size.sh &
#unzip boxImgTools
unzip /usr/sbin/boxImgTools.zip -d /tmp/bin/ &
