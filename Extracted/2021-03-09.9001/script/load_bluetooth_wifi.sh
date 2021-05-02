########################
# Copyright(c) 2014-2018 DongGuan HeWei Communication Technologies Co. Ltd.
# file    dbus_run.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    26Nov18
########################
#!/bin/bash
#load bluetooth
rtk_hciattach -s 115200 ttymxc2 rtk_h5

#load wifi
insmod /tmp/88x2bs.ko
