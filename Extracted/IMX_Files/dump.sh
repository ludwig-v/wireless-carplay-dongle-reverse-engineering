#!/bin/sh

dd if=/dev/mtd0 of=/mnt/UPAN/u-boot-env.img bs=64k seek=0 count=13
dd if=/dev/mtd1 of=/mnt/UPAN/zImage bs=64k seek=0 count=79
dd if=/dev/mtd2 of=/mnt/UPAN/imx6ul-14x14-evk.dtb bs=64k seek=0 count=1

exit 0