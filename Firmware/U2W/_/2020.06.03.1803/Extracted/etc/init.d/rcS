#!/bin/sh
PATH=/bin:/sbin:/usr/bin:/usr/sbin
#. /etc/profile
runlevel=S
prevlevel=N
umask 022
export PATH runlevel prevlevel
echo "----------------start ssh----------------"
mkdir /dev/pts
#dropbear
echo "---------------mount all--------------"
mount -a
echo "---------------Start mdev-------------"
echo /sbin/mdev>/proc/sys/kernel/hotplug
mdev -s
echo "-------------up network--------------"
ifconfig lo up
#ifconfig eth0 192.168.31.222 netmask 255.255.255.0 mtu 1500 up
#ifconfig eth0 up
#udhcpc -i eth0 &
echo "**************************************"
echo "Kernel version:linux-3.0.15"
echo "iMX6UL 14x14 evk board rootfs"
echo "Designer:ShiKai"
echo "Date:2015.11.3"
echo "**************************************"
/bin/hostname -F /etc/hostname
echo "Start Accessory driver!!!"
#/script/start_accessory.sh
echo Software Vserion: `cat /etc/software_version`
#ARMadb-driver &
echo "Start Carplay IAP2&NCM driver!!!"
/script/start_iap2_ncm.sh
echo "Start NCM network"
/script/start_ncm.sh
#echo "Start Mirror Service!!!"
#echo "Start Apple UsbNet Service!!!"
#/script/usbmuxd -U root -f &
echo "Start Carplay mdnsd!!!"
mdnsd
echo "Web Server Service!!!"
mkdir -p /tmp/cgi-bin/ && cp /etc/boa/cgi-bin/* /tmp/cgi-bin/
boa
#echo "Config mic for audio record!!!"
#/script/start_mic_record.sh

#echo "Start Carplay IAP2d and HNPd!!!"
#ARMiap2d &
#ARMiPhoneHNPd &
echo 1 > /proc/sys/net/ipv4/tcp_tw_reuse
echo 1 > /proc/sys/net/ipv4/tcp_tw_recycle       #for TIME_WAIT
echo 1 > /proc/sys/net/ipv4/tcp_orphan_retries   #for FIN_WAIT1
#for recv buf size
echo 16777216 > /proc/sys/net/core/rmem_max 
echo 2097152 > /proc/sys/net/core/rmem_default 
echo 16777216 > /proc/sys/net/core/wmem_max 
echo 2097152 > /proc/sys/net/core/wmem_default 
echo 4096 87380 16777216 > /proc/sys/net/ipv4/tcp_rmem
echo 4096 65536 16777216 > /proc/sys/net/ipv4/tcp_wmem
echo 10000000 10000000 10000000 > /proc/sys/net/ipv4/tcp_mem
echo 65536 > /proc/sys/net/core/netdev_max_backlog
echo 0 > /proc/sys/net/ipv4/tcp_timestamps
echo 0 > /proc/sys/net/ipv4/tcp_sack
#for max cpu freq
#echo performance >  /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
/script/start_iphone.sh &
