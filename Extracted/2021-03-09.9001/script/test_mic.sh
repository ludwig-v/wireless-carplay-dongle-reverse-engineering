########################
# Copyright(c) 2014-2017 DongGuan HeWei Communication Technologies Co. Ltd.
# file    update_box.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    03Jun17
########################
#!/bin/sh
dtmf_file=/tmp/dtmf.pcm
tinycap -- -c 1 -r 16000 -b 16 -t 4 > $dtmf_file
result=`dtmf_decode $dtmf_file | grep 14809414327 | wc -l`
if [ $result -eq 1 ] ; then
	echo "mic test success!!"
	exit 0
fi
exit 1
