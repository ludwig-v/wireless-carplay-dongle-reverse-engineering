########################
# Copyright(c) 2014-2015 DongGuan HeWei Communication Technologies Co. Ltd.
# file    mount-SK.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    12Jul15
########################
#!/bin/bash
killall -9 udhcpc
ifconfig eth2 up && udhcpc -i eth1 -T 1 -A 2 &
