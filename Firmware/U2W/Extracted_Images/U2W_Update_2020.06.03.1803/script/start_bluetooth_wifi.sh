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

#start wifi
ifconfig ncm0 down
echo 1 > /proc/sys/net/ipv6/conf/wlan0/disable_ipv6
ifconfig wlan0 192.168.50.2 netmask 255.255.255.0 mtu 1500 up
ps |grep hostapd |grep -v grep || hostapd /etc/hostapd.conf -B
ps |grep udhcpd |grep -v grep || udhcpd
echo opened > /tmp/wifi_status
