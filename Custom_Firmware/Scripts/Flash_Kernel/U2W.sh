#!/bin/sh

# turn off red light
echo 1 >/sys/class/gpio/gpio2/value;

# Flash partitions
## Warning: do not flash only zImage or only uBoot, you will brick the device, kernel start position is probably misaligned in partitions table.
dd if=/mnt/UPAN/uBoot of=/dev/mtdblock0 bs=128
dd if=/mnt/UPAN/zImage of=/dev/mtdblock1 bs=128

# turn on red light
echo 0 >/sys/class/gpio/gpio2/value;

exit 0
