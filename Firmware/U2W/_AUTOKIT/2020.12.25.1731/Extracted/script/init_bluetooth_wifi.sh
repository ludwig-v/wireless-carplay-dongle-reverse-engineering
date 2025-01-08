########################
# Copyright(c) 2014-2020 DongGuan HeWei Communication Technologies Co. Ltd.
# file    init_bluetooth_wifi.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    28 920
########################
#!/bin/bash

#insmod Wi-Fi module Driver
test -e /tmp/88x2bs.ko && insmod /tmp/88x2bs.ko

#attach bluetooth module
if [ -e /usr/sbin/rtk_hciattach ]; then
	echo "Start load Bluetooth!!!"
	rtk_hciattach -s 115200 ttymxc2 rtk_h5 &
fi

set_wifi_mac

