########################
# Copyright(c) 2014-2017 DongGuan HeWei Communication Technologies Co. Ltd.
# file    quickly_update.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    03Jun17
########################
#!/bin/sh
updatepath=/tmp/update
targerpath=/
smartcopy=/script/smart_copy.sh

#check flash space
/script/check_update.sh >> /dev/console || exit 1

cd $updatepath
for file in `find . -type f`; do
	$smartcopy $file $targerpath/$file
done
for file in `find . -type l`; do
	echo "copy link file: $file"
	cp -d $file $targerpath/$file
done
for file in `find . -type d`; do
	if [ ! -d $targerpath/$file ]; then
		echo "copy dir: $file"
		mkdir $targerpath/$file
	fi
done

if [ -e /tmp/copy_lib ]; then
	echo 6 > /tmp/update_status
	sleep 1
	killall ARMHiCar
	sleep 0.5
	cp /tmp/copy_lib/* /usr/lib/
	cp /tmp/copy_cgi/* /etc/boa/cgi-bin/
	sync
fi
