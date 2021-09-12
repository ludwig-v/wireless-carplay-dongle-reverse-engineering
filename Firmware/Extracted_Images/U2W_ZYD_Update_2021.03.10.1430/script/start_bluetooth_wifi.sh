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
LOCKFILE="/tmp/start_bluetooth_wifi_lock"
if [ -f ${LOCKFILE} ]
  then
    exit
  else
    touch ${LOCKFILE}
fi

#config wifi channel
if [ -e /etc/wifi_use_24G ]; then
	grep "^hw_mode=a" /etc/hostapd.conf && (sed -i "s/^hw_mode=a/hw_mode=g/" /etc/hostapd.conf;sed -i "s/^channel=36/channel=6/" /etc/hostapd.conf)
fi

#start wifi
echo 1 >/proc/sys/net/ipv6/conf/wlan0/disable_ipv6
ifconfig wlan0 192.168.50.2 netmask 255.255.255.0 mtu 1500 up
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
