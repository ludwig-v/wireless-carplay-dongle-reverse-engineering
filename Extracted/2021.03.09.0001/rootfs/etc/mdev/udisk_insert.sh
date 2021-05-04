########################
# Copyright(c) 2014-2017 DongGuan HeWei Communication Technologies Co. Ltd.
# file    udisk_insert.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    03Jun17
########################
#!/bin/sh
if [ -d /sys/block/*/$MDEV ] ; then
	echo "/dev/$MDEV PLUG IN" >> /dev/console
	mkdir -p /mnt/UPAN
	mount /dev/$MDEV /mnt/UPAN -t vfat -o rw,umask=0000,utf8=1
	blkid /dev/$MDEV | awk '{print $3}' > /tmp/udisk_uuid
	/script/update_box.sh /dev/$MDEV >> /dev/console &
fi

