########################
# Copyright(c) 2014-2015 DongGuan HeWei Communication Technologies Co. Ltd.
# file    mount-gadgetfs.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    14Jul15
########################
#!/bin/bash
#insmod  /lib/modules/3.4.79/kernel/drivers/usb/gadget/gadgetfs.ko
insmod  gadgetfs
mkdir /dev/gadget
mount -t gadgetfs none /dev/gadget/
