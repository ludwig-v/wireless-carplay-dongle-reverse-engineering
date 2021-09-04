#!/bin/sh

# turn off red light
echo 1 >/sys/class/gpio/gpio2/value;

# Copy nvram config file
cp /mnt/UPAN/nvram_bcm4354_m.txt /lib/firmware/bcm/nvram_bcm4354_m.txt
chmod 755 /lib/firmware/bcm/nvram_bcm4354_m.txt
sync

# turn on red light
echo 0 >/sys/class/gpio/gpio2/value;

exit 0
