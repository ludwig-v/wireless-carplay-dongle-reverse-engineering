########################
# Copyright(c) 2014-2022 DongGuan HeWei Communication Technologies Co. Ltd.
# file    check_log_size.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    1111月22
########################
#!/bin/bash
maxLogSize=10485760 #10M
logFile=/tmp/userspace.log
test -e /tmp/ttyLog && logFile=/tmp/ttyLog
while true
do
	logSize=`stat -c "%s" $logFile`
	echo "check_log_size: $logSize byte" >> /dev/console
	if [ $logSize -gt $maxLogSize ]; then
		tail -n 2000 $logFile > /tmp/.last.log
		cat /tmp/.last.log > $logFile
		rm -f /tmp/.last.log
		echo 3 > /proc/sys/vm/drop_caches
	fi
	sleep 30
done
