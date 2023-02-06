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
	dmesg | tail -n 1000 >> $lastLogPath
	tail -n 1000 /tmp/userspace.log >> $lastLogPath
	sync
}

#change USB to device mode
#test -e /usr/sbin/fakeiOSDevice && echo 1 > /sys/bus/platform/devices/ci_hdrc.1/inputs/a_bus_drop
#test -e /usr/sbin/fakeiOSDevice && echo 0 > /sys/bus/platform/devices/ci_hdrc.1/inputs/b_bus_req

saveLastLog /var/log/box_last_reboot.log

sync
df -h |grep /data && umount /data
/bin/umount -a -r
