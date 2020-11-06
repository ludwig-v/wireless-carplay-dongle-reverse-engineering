########################
# Copyright(c) 2014-2019 DongGuan HeWei Communication Technologies Co. Ltd.
# file    init_gpio.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    10May19
########################
#!/bin/bash
#set quickly charge mode
echo 6 > /sys/class/gpio/export;
echo out > /sys/class/gpio/gpio6/direction; 
echo 7 > /sys/class/gpio/export;
echo out > /sys/class/gpio/gpio7/direction;
echo 0 >/sys/class/gpio/gpio6/value;
sleep 0.1
echo 1 >/sys/class/gpio/gpio6/value;
echo 1 >/sys/class/gpio/gpio7/value;

#reset BT
ifconfig wlan0 up
echo 1 > /sys/class/gpio/export;
echo out > /sys/class/gpio/gpio1/direction; 
echo 1 >/sys/class/gpio/gpio1/value;
sleep 0.1
echo 0 >/sys/class/gpio/gpio1/value;
ifconfig wlan0 down

#Power LED ON
echo 2 > /sys/class/gpio/export;
echo out > /sys/class/gpio/gpio2/direction;
echo 0 >/sys/class/gpio/gpio2/value;
