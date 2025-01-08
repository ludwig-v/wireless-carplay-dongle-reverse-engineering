########################
# Copyright(c) 2014-2019 DongGuan HeWei Communication Technologies Co. Ltd.
# file    init_hicar_lib.sh 
# brief   
# author  Shi Kai
# version 1.0.0
# date    10May19
########################
#!/bin/bash
#cp so and exe to tmp, save space of ROM for update
tmpLibPath=/tmp/lib/
tmpBinPath=/tmp/bin/
mkdir -p $tmpLibPath
mkdir -p $tmpBinPath


cp -d /usr/lib/libcrypto.so* $tmpLibPath

cp /usr/sbin/ARMadb-driver $tmpBinPath

#tar decompress koS to /tmp
tar -xvf /script/ko.tar.gz -C /tmp

#cp -d /usr/lib/lib*so* $tmpLibPath
#for file in `find /usr/sbin -type f`; do
#	cp $file $tmpBinPath
#done	
