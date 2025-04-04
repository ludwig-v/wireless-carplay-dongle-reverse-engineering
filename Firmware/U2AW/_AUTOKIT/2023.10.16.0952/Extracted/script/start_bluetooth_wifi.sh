########################
# Copyright(c) 2014-2018 DongGuan HeWei Communication Technologies Co. Ltd.
# file    dbus_run.sh
# brief
# author  Shi Kai
# version 1.0.0
# date    26Nov18
########################
#!/bin/bash
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

setBrandIp() {
    confPath="/etc/udhcpd.conf"
    brandIp=$1
    ip=${brandIp%.*}
	echo "Set WLAN0 IP: $ip "
    if [ `grep -c "$ip.100" $confPath` -ne '0' ]; then
        echo "Same IP, no need to reset"
        return 0
    else
        sed -i "s/192.168.*.100/${ip}.100/" $confPath
        sed -i "s/192.168.*.200/${ip}.200/" $confPath
    fi
}

#LOCK
LOCKFILE="/tmp/run_bluetooth_wifi_lock"
if [ -f ${LOCKFILE} ]
  then
	echo "run_bluetooth_wifi_lock, exit!"
    exit
  else
    touch ${LOCKFILE}
fi

#wifi param
ssid="`cat /etc/wifi_name`"
passwd="`riddleBoxCfg -g WifiPassword`"

#config wifi channel
if [ -e /etc/wifi_use_24G ]; then
	riddleBoxCfg -s WiFiChannel 6
fi
wifi_channel=`riddleBoxCfg -g WiFiChannel`
freq=5180
if [ $wifi_channel -gt 15 ]; then
	let freq=5000+wifi_channel*5
else
	let freq=2407+wifi_channel*5
fi
echo "config wifi channel: $wifi_channel, freq: $freq"
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
grep "^wpa_passphrase=${passwd}$" /etc/hostapd.conf || sed -i "s/^wpa_passphrase=.*/wpa_passphrase=${passwd}/" /etc/hostapd.conf

#check use ap or p2p, set p2p params
mode=AP
if [ $# -ge 1 ];then
	mode=$1
	ssid="DIRECT-`cat /etc/wifi_name|sed 's/ //g'`"
fi
if [ $# -ge 2 ];then
	ssid=$2
fi
if [ $# -ge 3 ];then
	passwd=$3
fi
if [ $# -ge 4 ];then
	freq=$4
fi

curMode=AP
if [ -e /usr/sbin/wpa_cli ]; then
	wpa_cli -i wlan0 list_network && curMode=P2P
fi

#check mode changed
if [ "$mode" != "$curMode" ]; then
	echo "Change WiFi mode from $curMode to $mode"
	rm -f ${LOCKFILE}
	/script/close_bluetooth_wifi.sh
	touch ${LOCKFILE}
fi
 
#start wifi
if [ "$mode" == "AP" ]; then
	test -e /tmp/bin/hostapd || cp /usr/sbin/hostapd /tmp/bin/hostapd
	startprocess hostapd 'hostapd /etc/hostapd.conf -B'
else
	echo "Use P2P as AP"
	test -e /tmp/bin/wpa_cli || cp /usr/sbin/wpa_cli /tmp/bin/
	wpa_cli -i wlan0 list_network || (iw dev wlan0 del;wpa_cli p2p_group_add ssid=$ssid passphrase=$passwd freq=$freq)
fi
if [[ $? == 0 ]];then
echo opened >/tmp/wifi_status
fi

WLANIP=192.168.50.2
test -e /usr/sbin/ARMHiCar && WLANIP=192.168.43.1
#test -e /etc/box_ip && WLANIP=`cat /etc/box_ip`
BoxIp="`riddleBoxCfg -g BoxIp`"
if [ -n "$BoxIp" ]; then
	WLANIP=$BoxIp
fi
setBrandIp $WLANIP

echo 1 >/proc/sys/net/ipv6/conf/wlan0/disable_ipv6
ifconfig wlan0 $WLANIP netmask 255.255.255.0 mtu 1500 up

#ensure start uhhcpd
startprocess udhcpd 'udhcpd'

#ensure start mdnsd
#startprocess mdnsd 'mdnsd'

#UNLOCK
if [ -f ${LOCKFILE} ]
  then
    rm -rf ${LOCKFILE}
fi
