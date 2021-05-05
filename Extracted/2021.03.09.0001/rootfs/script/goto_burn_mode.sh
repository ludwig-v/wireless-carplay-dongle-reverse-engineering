#!/bin/sh
flash_erase /dev/mtd0 0 1 && echo "Clear boot data success!"
reboot
