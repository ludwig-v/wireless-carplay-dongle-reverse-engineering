########################
# Copyright(c) 2014-2015 DongGuan HeWei Communication Technologies Co. Ltd.
# file    mount-SK.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    12Jul15
########################
#!/bin/bash
if [ $# -eq 2 ]; then
	if [ -e $1 -a -e $2 ]; then
		file1_md5=`md5sum $1 | awk '{print $1}'`
		file2_md5=`md5sum $2 | awk '{print $1}'`
		#echo $file1_md5
		#echo $file2_md5
		if [[ $file1_md5 == $file2_md5 ]]; then
			echo "$1 and $2 is same, not need copy!!!"
			exit 0
		fi
	fi
	echo "cp $1 $2"
	cp $1 $2
fi

