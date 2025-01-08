########################
# Copyright(c) 2014-2020 DongGuan HeWei Communication Technologies Co. Ltd.
# file    once.sh
# brief   
# author  Haiguang Yin
# version 1.0.0
# date    14Apr20
########################
#!/bin/bash

setUpdateProgress() {
	echo $1 > /tmp/update_progress
}

#TODO 避免不带加密IC的板子无法升级，将来需要删除掉这段脚本
test -e /etc/manufacture_date || echo -n `awk -F. '{print $1$2$3}' /etc/software_version` > /etc/manufacture_date

/tmp/update/tmp/remove_unnecessary_file.sh

setUpdateProgress 6

# Changing the 'riddle_default. conf' after upgrading dose not need to resetting the box
if [ -e /tmp/update/etc/riddle_default.conf ]; then
	diff /etc/riddle_default.conf /tmp/update/etc/riddle_default.conf
	result=$?
	if [ "$result" -ne 0 ]; then
		cp /tmp/update/etc/riddle_default.conf /etc/
		cp /tmp/update/usr/sbin/riddleBoxCfg /usr/sbin/
		riddleBoxCfg --upConfig
	fi
fi	

# for update new hicar sdk
#cat /etc/software_version |grep "2022.11" && rm -f /etc/hichain/extention_component.dat /etc/hichain/external_component.dat /etc/hichain/deviceInfo.txt
# for update new hicar model id
versionNum=`cat /etc/software_version |sed s/'\.'//g`
if [ $versionNum -lt "202304251700" ] && [ -e /etc/hichain/hks_keystore ]; then
	cat /etc/box_product_type |grep "U2H" || (rm -f /etc/hichain/* /etc/bluetooth/mybluetooth*;sync)
fi
/script/phone_link_deamon.sh CarPlay stop

#TODO, update boa in upg
wwwPkg=/tmp/update/etc/boa/www/www.tar.gz
if [ -e $wwwPkg ]; then
	rm -rf /etc/boa/www/*
	test -e $wwwPkg && (tar -xvf $wwwPkg -C /etc/boa/www/;rm -f $wwwPkg)
	cp -r /tmp/update/etc/boa/www/* /etc/boa/www/
	cp -r /tmp/update/etc/boa/cgi-bin/* /etc/boa/cgi-bin/
	sync
fi
setUpdateProgress 7

sdioCardID=`cat /sys/bus/sdio/devices/mmc0:0001:1/device`
if [ "$sdioCardID" == "0xb822" ]; then
	if [ -e /tmp/update/rtl8822_rootfs.tar.gz ]; then
		tar -xvf /tmp/update/rtl8822_rootfs.tar.gz -C /tmp/update/
	fi
elif [ "$sdioCardID" == "0x4354" ] || [ "$sdioCardID" == "0x4335" ]; then
	if [ -e /tmp/update/bcm4354_rootfs.tar.gz ]; then
		tar -xvf /tmp/update/bcm4354_rootfs.tar.gz -C /tmp/update/
		if [ "$sdioCardID" == "0x4335" ]; then 
			mv /tmp/update/lib/firmware/bcm/4335/* /tmp/update/lib/firmware/bcm/
		fi
		rm -rf /tmp/update/lib/firmware/bcm/4335
	else
		if [ "$sdioCardID" == "0x4354" ]; then 
			rm -rf /tmp/update/lib/firmware/bcm/4335 /lib/firmware/bcm/4335
		fi
	fi
elif [ "$sdioCardID" == "0x9149" ] || [ "$sdioCardID" == "0x9141" ]; then
	if [ -e /tmp/update/sd8987_rootfs.tar.gz ]; then
		tar -xvf /tmp/update/sd8987_rootfs.tar.gz -C /tmp/update/
	fi
elif [ "$sdioCardID" == "0xc822" ]; then
	if [ -e /tmp/update/rtl8822cs_rootfs.tar.gz ]; then
		tar -xvf /tmp/update/rtl8822cs_rootfs.tar.gz -C /tmp/update/
	fi
elif [ "$sdioCardID" == "0x4358" ] || [ "$sdioCardID" == "0xaa31" ]; then
	if [ -e /tmp/update/bcm4358_rootfs.tar.gz ]; then
		tar -xvf /tmp/update/bcm4358_rootfs.tar.gz -C /tmp/update/
	fi
elif [ "$sdioCardID" == "0xb733" ]; then
	if [ -e /tmp/update/rtl8733_rootfs.tar.gz ]; then
		tar -xvf /tmp/update/rtl8733_rootfs.tar.gz -C /tmp/update/
	fi  
fi
setUpdateProgress 8
rm -f /tmp/update/rtl8822_rootfs.tar.gz
rm -f /tmp/update/bcm4354_rootfs.tar.gz
rm -f /tmp/update/sd8987_rootfs.tar.gz
rm -f /tmp/update/rtl8822cs_rootfs.tar.gz
rm -f /tmp/update/bcm4358_rootfs.tar.gz
rm -f /tmp/update/rtl8733_rootfs.tar.gz
test -e /etc/wifi_name && sed -i "s/^ssid=.*/ssid=`cat /etc/wifi_name`/" /tmp/update/etc/hostapd.conf
setUpdateProgress 9

echo "Start upg"
rm /tmp/*.img;rm /tmp/update.tar.gz
mv /tmp/update/tmp/upg /tmp/;mv /tmp/update/tmp/*.img /tmp/
chmod +x /tmp/upg
/tmp/upg >> /dev/console 2>&1 || exit 1
echo "End upg"
