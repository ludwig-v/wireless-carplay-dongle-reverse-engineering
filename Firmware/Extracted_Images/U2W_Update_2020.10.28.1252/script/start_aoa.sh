########################
# Copyright(c) 2014-2015 DongGuan HeWei Communication Technologies Co. Ltd.
# file    mount-SK.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    12Jul15
########################
#!/bin/bash
echo 0 > /sys/class/android_usb_accessory/android0/enable
#echo "Magic Communication Tec." > /sys/class/android_usb_accessory/android0/iManufacturer
#echo "Auto Box" > /sys/class/android_usb_accessory/android0/iProduct
echo 18d1 > /sys/class/android_usb_accessory/android0/idVendor
echo 2d00 > /sys/class/android_usb_accessory/android0/idProduct
echo accessory > /sys/class/android_usb_accessory/android0/functions
echo 1 > /sys/class/android_usb_accessory/android0/enable
