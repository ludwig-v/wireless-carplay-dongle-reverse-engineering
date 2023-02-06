########################
# Copyright(c) 2014-2017 DongGuan HeWei Communication Technologies Co. Ltd.
# file    check_update.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    03Jun17
########################
#!/bin/sh
updatepath=/tmp/update
targetpath=/

totalSize=0
leftSize=`df -B 1 | grep dev/root | awk '{print $4}'`

tmpFileList=""
tmpLibPath=/tmp/lib/
tmpBinPath=/tmp/bin/
test -d $tmpBinPath || tmpFileList="./usr/sbin/fakeiOSDevice,./usr/sbin/ui.tar.gz,./usr/sbin/ARMadb-driver,./usr/sbin/AppleCarPlay"
test -d $tmpLibPath || tmpLibPath=/tmp/hicar_lib
for file in `find $tmpLibPath -type f`; do
	tmpFileList="$tmpFileList,./usr/lib/`basename $file`"
done
for file in `find $tmpBinPath -type f`; do
	tmpFileList="$tmpFileList,./usr/sbin/`basename $file`"
done

cd $updatepath
for file in `find . -type f`; do
	if [ `dirname $file` == "./tmp" ]; then
		continue
	fi
	diff $file $targetpath/$file >> /dev/null
	if [ $? -ne 0 ]; then
		size=`stat -c "%s" $file`
		let totalSize+=$size
		echo "$file size: $size, need update"
		echo $tmpFileList | grep $file && let leftSize+=`stat -c "%s" $targetpath/$file`
	fi
done

echo "leftSize: $leftSize"
echo "totalSize: $totalSize"



if [ $totalSize -gt $leftSize ]; then
	echo "Flash left size not enough, Can't update!!!"
	exit 1
else
	exit 0
fi
