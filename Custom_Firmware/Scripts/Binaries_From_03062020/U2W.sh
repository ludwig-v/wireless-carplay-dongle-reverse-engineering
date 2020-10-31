#!/bin/sh

killall -9 ARMadb-driver
killall -9 AppleCarPlay
killall -9 fakeiOSDevice
killall -9 boa
rm -rf /usr/sbin/AppleCarPlay
rm -rf /usr/sbin/ARMadb-driver
rm -rf /usr/sbin/fakeiOSDevice
rm -rf /etc/boa/cgi-bin/server.cgi
rm -rf /etc/boa/cgi-bin/upload.cgi
cp /mnt/UPAN/AppleCarPlay /usr/sbin/AppleCarPlay
cp /mnt/UPAN/ARMadb-driver /usr/sbin/ARMadb-driver
cp /mnt/UPAN/fakeiOSDevice /usr/sbin/fakeiOSDevice
cp /mnt/UPAN/server.cgi /etc/boa/cgi-bin/server.cgi
cp /mnt/UPAN/upload.cgi /etc/boa/cgi-bin/upload.cgi
rm -rf /mnt/UPAN/U2W.sh # Auto delete
reboot