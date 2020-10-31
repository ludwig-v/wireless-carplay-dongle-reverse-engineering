#!/bin/sh

# Dump rootFS files
mkdir -p /mnt/UPAN/Dump
mkdir -p /mnt/UPAN/Dump/Library && cp -R /Library/* /mnt/UPAN/Dump/Library
mkdir -p /mnt/UPAN/Dump/bin && cp -R /bin/* /mnt/UPAN/Dump/bin
mkdir -p /mnt/UPAN/Dump/etc && cp -R /etc/* /mnt/UPAN/Dump/etc
mkdir -p /mnt/UPAN/Dump/lib && cp -R /lib/* /mnt/UPAN/Dump/lib
mkdir -p /mnt/UPAN/Dump/root && cp -R /root/* /mnt/UPAN/Dump/root
mkdir -p /mnt/UPAN/Dump/sbin && cp -R /sbin/* /mnt/UPAN/Dump/sbin
mkdir -p /mnt/UPAN/Dump/script && cp -R /script/* /mnt/UPAN/Dump/script
mkdir -p /mnt/UPAN/Dump/usr && cp -R /usr/* /mnt/UPAN/Dump/usr
mkdir -p /mnt/UPAN/Dump/var && cp -R /var/* /mnt/UPAN/Dump/var
