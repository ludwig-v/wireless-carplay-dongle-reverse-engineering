########################
# Copyright(c) 2014-2018 DongGuan HeWei Communication Technologies Co. Ltd.
# file    dbus_run.sh
# brief
# author  Shi Kai
# version 1.0.0
# date    26Nov18
########################
#!/bin/bash
#close bluetooth
#hciconfig hci0 down
#killall dbus-daemon
#killall hcid
#echo closed > /tmp/bluetooth_status
#rm -f /tmp/auto_connect_status

EnsureProcessKill() {
    killall $1
    i=1
    while true
    do  
        ProcNumber=`ps |grep -w $1|grep -v grep|wc -l`
        if [ $ProcNumber -le 0 ];then
           break
        else
           sleep 0.2
           killall $1
        fi
    done
}

#close wifi
ifconfig wlan0 down
ifconfig ncm0 up
EnsureProcessKill hostapd
EnsureProcessKill udhcpd

echo closed > /tmp/wifi_status
rm -f /tmp/wifi_connection_list

