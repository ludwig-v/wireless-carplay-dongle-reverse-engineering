########################
# Copyright(c) 2014-2019 DongGuan HeWei Communication Technologies Co. Ltd.
# file    set_quick_charge_mode.sh
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
echo 1 > /sys/class/gpio/export;
echo out > /sys/class/gpio/gpio1/direction; 
echo 1 >/sys/class/gpio/gpio1/value;
sleep 0.1
echo 0 >/sys/class/gpio/gpio1/value;

#Power LED ON
#echo 2 > /sys/class/gpio/export;
#echo out > /sys/class/gpio/gpio2/direction;
#echo 0 >/sys/class/gpio/gpio2/value;
