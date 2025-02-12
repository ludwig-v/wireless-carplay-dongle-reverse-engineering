#!/bin/sh

# turn off red light

echo 1 >/sys/class/gpio/gpio2/value;

# copy debug tools

cp /mnt/UPAN/gdb /tmp/bin
cp /mnt/UPAN/gdbserver /tmp/bin
cp /mnt/UPAN/strace /tmp/bin
chmod -R 755 /tmp/bin

# turn on red light

echo 0 >/sys/class/gpio/gpio2/value;

exit 0
