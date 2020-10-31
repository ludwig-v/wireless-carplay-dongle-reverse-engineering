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

#close wifi
ifconfig wlan0 down
ifconfig ncm0 up
killall hostapd
killall udhcpd
echo closed > /tmp/wifi_status
