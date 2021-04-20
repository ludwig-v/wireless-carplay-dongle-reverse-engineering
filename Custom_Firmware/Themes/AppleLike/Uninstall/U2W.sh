#!/bin/sh

# turn off red light
echo 1 >/sys/class/gpio/gpio2/value;

#Move ui.tar.gz from /usr/sbin/ to USB as ui.tar.gz.carplaytheme
mv /usr/sbin/ui.tar.gz /mnt/UPAN/ui.tar.gz.carplaytheme

#Copy Stock Theme from USB to /usr/sbin
cp /mnt/UPAN/ui.tar.gz.original /usr/sbin/ui.tar.gz
chmod 775 /usr/sbin/ui.tar.gz
sync

# turn on red light
echo 0 >/sys/class/gpio/gpio2/value;
exit 0