########################
# Copyright(c) 2014-2015 DongGuan HeWei Communication Technologies Co. Ltd.
# file    smart_copy.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    12Jul15
########################
#!/bin/bash
if [ $# -eq 2 ]; then
	diff $1 $2 >> /dev/null
	if [ $? -eq 0 ]; then
		echo "$1 and $2 is same, not need copy!!!"
		exit 0
	fi
	file2_dir=`dirname $2`
	if [ ! -d $file2_dir ]; then
		mkdir -p $file2_dir
	fi
	echo "cp $1 $2"
	cp $1 $2
fi

