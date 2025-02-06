#!/bin/sh

# turn off red light

echo 1 >/sys/class/gpio/gpio2/value;

# copy dropbear

cp /mnt/UPAN/dropbear /tmp
chmod 775 /tmp/dropbear

# setup stp-server

cp /mnt/UPAN/sftp-server /tmp
chmod 775 /tmp/sftp-server

mkdir /usr/libexec
ln -s /tmp/sftp-server /usr/libexec/sftp-server 

cp /mnt/UPAN/libz.so.1.2.11 /tmp
ln -s /tmp/libz.so.1.2.11 /usr/lib/libz.so 

# copy public key

mkdir /root
mkdir /root/.ssh
cp /mnt/UPAN/cplay2air.pub /root/.ssh/authorized_keys
chmod 700 /root
chmod 700 /root/.ssh
chmod 600 /root/.ssh/authorized_keys
chown root:root /root

# launch dropbear

/tmp/dropbear -vFEsg

# turn on red light

echo 0 >/sys/class/gpio/gpio2/value;

exit 0
