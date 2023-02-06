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
/script/check_update.sh >> /dev/ttymxc0 || exit 1

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
