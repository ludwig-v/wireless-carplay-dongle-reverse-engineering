########################
# Copyright(c) 2014-2020 DongGuan HeWei Communication Technologies Co. Ltd.
# file    after_shutdown.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    27Feb20
########################
#!/bin/bash

saveLastLog() {
	lastLogPath=$1
	#if [ -e $lastLogPath ]; then
	#	return
	#fi
	echo "Save last log when reboot" > $lastLogPath
	df -h >> $lastLogPath
	cat /proc/meminfo >> $lastLogPath
	ps -l >> $lastLogPath
	echo y > /sys/module/printk/parameters/time
	dmesg >> $lastLogPath
	tail -n 1000 /tmp/userspace.log >> $lastLogPath
	sync
}

saveLastLog /var/log/box_last_reboot.log

sync
/bin/umount -a -r
