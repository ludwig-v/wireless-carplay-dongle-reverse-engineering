#!/bin/sh

# turn off red light

echo 1 >/sys/class/gpio/gpio2/value;

# copy dropbear

cp /mnt/UPAN/dropbear /tmp
chmod 775 /tmp/dropbear

ln -s /lib/ld-linux.so.3 /lib/ld-linux-armhf.so.3

# copy public key

mkdir /root
mkdir /root/.ssh
cp /mnt/UPAN/cplay2air.pub /root/.ssh/authorized_keys
chmod 700 /root/.ssh
chmod 600 /root/.ssh/authorized_keys
chown root:root /root

# launch dropbear

/tmp/dropbear -vFEsg

# turn on red light

echo 0 >/sys/class/gpio/gpio2/value;

exit 0
