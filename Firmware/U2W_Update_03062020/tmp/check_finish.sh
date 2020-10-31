########################
# Copyright(c) 2014-2017 DongGuan HeWei Communication Technologies Co. Ltd.
# file    check_finish.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    03Jun17
########################
#!/bin/sh
echo "Run $0 start!!!" >> /dev/ttymxc0
while [ `cat /tmp/update_status` != "2" ]; do
	sleep 1
done

echo 2 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio2/direction
echo 0 >/sys/class/gpio/gpio2/value

echo 9 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio9/direction
echo 0 >/sys/class/gpio/gpio9/value

