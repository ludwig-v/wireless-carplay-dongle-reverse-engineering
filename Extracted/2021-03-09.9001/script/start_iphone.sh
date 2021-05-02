########################
# Copyright(c) 2014-2015 DongGuan HeWei Communication Technologies Co. Ltd.
# file    mount-SK.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    12Jul15
########################
#!/bin/bash
#init gpio
/script/init_gpio.sh

cp /usr/sbin/fakeiOSDevice /tmp
cd /tmp 
if [ -e /etc/log_com ]; then
./fakeiOSDevice 2>&1 | tee /tmp/userspace.log &
elif [ -e /etc/log_file ]; then
./fakeiOSDevice > /tmp/userspace.log 2>&1 &
else
./fakeiOSDevice >> /dev/null 2>&1 &
fi

#start bluetooth
rtk_hciattach -s 115200 ttymxc2 rtk_h5 &

#check reboot
# while true
# do
# 	sleep 2
# 	ps | grep -v grep | grep fakeiOSDevice >> /dev/null || reboot
# done

