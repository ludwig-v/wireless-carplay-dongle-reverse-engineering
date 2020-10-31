#!/bin/sh
flash_erase /dev/mtd2 0 1 && dd if=/tmp/imx6ul-14x14-evk.dtb of=/dev/mtd2 bs=64k seek=0 count=1 && echo "Update dtb Success!" || exit 1
