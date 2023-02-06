########################
# Copyright(c) 2014-2018 DongGuan HeWei Communication Technologies Co. Ltd.
# file    dbus_run.sh
# brief
# author  Shi Kai
# version 1.0.0
# date    26Nov18
########################
#!/bin/bash
#start bluetooth
#mkdir -p /var/run/dbus/
#rm -f /var/run/dbus/pid
#dbus-daemon --system --print-pid --print-address > /tmp/dbus_system
#hciconfig hci0 up
#hcid -x -n -m 8196&
#hcid -x -m 8196
#sed -i "s/001107/001106/" /etc/bluetooth/eir_info
#sleep 0.5 && sdptool add IAP2
#sleep 0.5 && hciconfig hci0 inqdata `cat /etc/bluetooth/eir_info`
#echo opened > /tmp/bluetooth_status
#ps | grep aaa|grep -v grep; if [ $? -ne 0 ]	then exit(0) else exit(1) fi;
startprocess() {
    ProcNumber=`ps |grep -w $1|grep -v grep|wc -l`
    if [ $ProcNumber -le 0 ];then
        echo $1 "start process.....";$2
        return 0
    else
    	echo $1 "runing....."
    fi
    return 1
}

#LOCK
LOCKFILE="/tmp/run_bluetooth_wifi_lock"
if [ -f ${LOCKFILE} ]
  then
    exit
  else
    touch ${LOCKFILE}
fi

#config wifi channel
wifi_channel=`riddleBoxCfg -g WiFiChannel`
if [ -e /etc/wifi_use_24G ]; then
	wifi_channel=6
fi
echo "config wifi channel: $wifi_channel"
if [ $wifi_channel -ge 1 ] && [ $wifi_channel -le 14 ]; then
	echo "2.4G wifi"
	grep "^hw_mode=a" /etc/hostapd.conf && sed -i "s/^hw_mode=a/hw_mode=g/" /etc/hostapd.conf
elif [ $wifi_channel -ge 34 ] && [ $wifi_channel -le 165 ]; then
	echo "5G wifi"
	grep "^hw_mode=g" /etc/hostapd.conf && sed -i "s/^hw_mode=g/hw_mode=a/" /etc/hostapd.conf
else
	echo "Default wifi channel 36"
	wifi_channel=36
	grep "^hw_mode=g" /etc/hostapd.conf && sed -i "s/^hw_mode=g/hw_mode=a/" /etc/hostapd.conf
fi
grep "^channel=${wifi_channel}$" /etc/hostapd.conf || sed -i "s/^channel=.*/channel=${wifi_channel}/" /etc/hostapd.conf
 

#start wifi
WLANIP=192.168.50.2
test -e /usr/sbin/ARMHiCar && WLANIP=192.168.43.1
echo 1 >/proc/sys/net/ipv6/conf/wlan0/disable_ipv6
ifconfig wlan0 $WLANIP netmask 255.255.255.0 mtu 1500 up
ifconfig ncm0 down
startprocess hostapd 'hostapd /etc/hostapd.conf -B'
if [[ $? == 0 ]];then
echo opened >/tmp/wifi_status
fi
#ensure start uhhcpd
startprocess udhcpd 'udhcpd'

#ensure start mdnsd
startprocess mdnsd 'mdnsd'

#UNLOCK
if [ -f ${LOCKFILE} ]
  then
    rm -rf ${LOCKFILE}
fi
