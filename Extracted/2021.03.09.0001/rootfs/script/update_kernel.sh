#!/bin/sh
flash_erase /dev/mtd1 0 79 && dd if=/tmp/zImage of=/dev/mtd1 bs=64k seek=0 count=79 && echo "Update kernel Success!" || exit 1
