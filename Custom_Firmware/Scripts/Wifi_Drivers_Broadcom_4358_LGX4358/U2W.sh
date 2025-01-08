#!/bin/sh

# turn off red light

echo 1 >/sys/class/gpio/gpio2/value;

# remove any old driver
rm -rf /rtl8822cs_rootfs.tar.gz
rm -rf /rtl8822_rootfs.tar.gz
rm -rf /rtl8733_rootfs.tar.gz
rm -rf /bcm4354_rootfs.tar.gz
rm -rf /bcm4358_rootfs.tar.gz
rm -rf /sd8987_rootfs.tar.gz

rm -rf /lib/firmware/rtlbt*
rm -rf /usr/sbin/rtk_*
rm -rf /lib/firmware/bcm*
rm -rf /usr/sbin/brcm*
rm -rf /usr/sbin/wl
rm -rf /lib/firmware/nxp*

# copy wifi driver
cp /mnt/UPAN/bcm4358_rootfs.tar.gz /tmp/bcm4358_rootfs.tar.gz
tar -xvf /tmp/bcm4358_rootfs.tar.gz -C /
rm -rf /tmp/bcm4358_rootfs.tar.gz

sync

# turn on red light

echo 0 >/sys/class/gpio/gpio2/value;