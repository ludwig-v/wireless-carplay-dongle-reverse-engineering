#!/bin/sh

# turn off red light
echo 1 >/sys/class/gpio/gpio2/value;

#Copy Original Theme to USB for Uninstall
mv /usr/sbin/ui.tar.gz /mnt/UPAN/ui.tar.gz.original

#Copy ui.tar.gz to /usr/sbin
cp /mnt/UPAN/ui.tar.gz /usr/sbin
chmod 775 /usr/sbin/ui.tar.gz
sync

# turn on red light
echo 0 >/sys/class/gpio/gpio2/value;
exit 0