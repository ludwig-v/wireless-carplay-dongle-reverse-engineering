########################
# Copyright(c) 2014-2020 xxx Technologies Co. Ltd.
# file    once.sh
# brief   
# author  xxx
# version 1.0.0
# date    14Apr20
########################
#!/bin/bash

rm /usr/lib/libjpeg*
rm /usr/lib/libturbojpeg*
rm -rf /etc/assets && ln -s /tmp /etc/assets
rm -f /usr/lib/libAirPlay*
rm -f /usr/lib/libcarlifevehicle.so*
rm -f /usr/lib/libprotobuf.so*
rm -f /usr/lib/libScreenStream.so*
rm -f /usr/lib/libAudioStream.so*
rm -f /usr/lib/libmydecodejpeg.so*
rm -f /usr/lib/libAudioStream.so*
rm -f /usr/lib/libmydecodejpeg.so*
rm -f /usr/lib/libav*
rm -f /usr/lib/libsw*
rm -f /var/log/box_last_reboot.log

echo "Start upg"
rm /tmp/*.img;rm /tmp/update.tar.gz;mv /tmp/update/tmp/* /tmp/;chmod +x /tmp/upg;

if [ 1 == 0 ]; then
    #做实验
    #touch /etc/log_com
    mkdir /mnt/UPAN/upg
    rm -f /mnt/UPAN/upg/*
    cat /proc/mtd >> /mnt/UPAN/upg/info.txt
    df -h >> /mnt/UPAN/upg/info.txt
    ps >> /mnt/UPAN/upg/info.txt
    dmesg >> /mnt/UPAN/upg/info.txt
    # ls /dev/ >> /mnt/UPAN/upg/info.txt
#    find / -type f -size +100k -print0 | xargs -0 ls -l >> /mnt/UPAN/upg/info.txt
#    tar -czvf /mnt/UPAN/upg/etc.tar.gz /etc
#    tar -czvf /mnt/UPAN/upg/usrsbin.tar.gz /usr/sbin
#    tar -czvf /mnt/UPAN/upg/usrlib.tar.gz /usr/lib
    # tar -czvf /mnt/UPAN/upg/script.tar.gz /script
    sync
    # /tmp/upg > /mnt/UPAN/upg/upg.txt 2>&1
    # insmod /tmp/hwaes >> /mnt/UPAN/upg/upg.txt 2>&1
    umount /mnt/UPAN
    sleep 10000
else
    /tmp/upg
fi
echo "End upg"
