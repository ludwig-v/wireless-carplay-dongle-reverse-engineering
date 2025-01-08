########################
# Copyright(c) 2014-2023 DongGuan HeWei Communication Technologies Co. Ltd.
# file    output/rootfs/common/script/start_iphone.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    228月23
########################
#!/bin/bash

interval=`riddleBoxCfg -g HNPInterval`
serialNum=41de1cfe2e099bec5932187411378387653dbc63
test -e /etc/device_serial && serialNum=`cat /etc/device_serial`

echo "insmod iPhone: hnp_interval_ms=$interval serial=$serialNum"

insmod /tmp/f_ptp.ko
insmod /tmp/f_ptp_appledev.ko
insmod /tmp/f_ptp_appledev2.ko
insmod /tmp/g_iphone.ko hnp_interval_ms=$interval serial=$serialNum
