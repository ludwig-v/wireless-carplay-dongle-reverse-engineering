########################
# Copyright(c) 2014-2023 DongGuan HeWei Communication Technologies Co. Ltd.
# file    finish.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    301月23
########################
#!/bin/bash


saveUpdateLog() {
	lastLogPath=$1
	echo "Save update log" > $lastLogPath
	df -h >> $lastLogPath
	cat /proc/meminfo >> $lastLogPath
	ps -l >> $lastLogPath
	echo y > /sys/module/printk/parameters/time
	dmesg | tail -n 500 >> $lastLogPath
	tail -n 1000 /tmp/userspace.log >> $lastLogPath
	sync
}

test -e /etc/riddle_special.conf && riddleBoxCfg --specialConfig

df -h |grep /var/run || rm -rf /var/run/*

saveUpdateLog /var/log/box_update.log
sync

