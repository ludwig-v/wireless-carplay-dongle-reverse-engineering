
![Carlinkit V2](https://i.imgur.com/ZL3dq41.png)

## Carlinkit / Carplay2Air Reverse Engineering

## Hardware

| Hardware | Part |
|--|--|
| Flash | Macronix 25L12835F (16MB) |
| SoC | Freescale i.MX6 UltraLite |
| CPU | ARM Cortex-A7 (ARMv7) |
| RAM | Micron/SK hynix 1Gb (64x16)
| Wi-Fi/BT | Realtek RTL8822BS or Fn-Link L287B-SR or Broadcom BCM4354 |

## Hardware differences

Hardware base remains almost exactly the same since the beginning, only device case / design changed (Thermal pads on V4 / Holes for air flow on V5)

Wifi chip only seems to be changed depending on the stock of the manufacturer. Whatever Wifi chip is being used, is connected over SDIO Protocol (High Speed 4-Bit SD) at 50Mhz, for a maximum speed of 25MB/s (200 Mbps)

Carlinkit created its own segmentation mainly for marketing using different softwares:
- V1 / V2 / V3 : Only Carplay Wired OEM for Wireless Carplay
- V4 : Only Carplay Wired OEM for Wireless Carplay + Wireless Android Auto
- V5: Carplay Wired OEM or Android Auto Wired OEM for both Wireless Carplay and Wireless Android Auto

Fun fact: It is possible to switch from one to another by rewriting the flash chip (V5 > V3, V3 > V4, and so on). Please note that Carlinkit controls devices activation so it may not work in the future so please save your original flash

# Software

`2021.03.09.0001` on CPLAY2Air:

```bash
$ cat /proc/cmdline
console=ttyLogFile0 root=/dev/mtdblock2 rootfstype=jffs2 mtdparts=21e0000.qspi:256k(uboot),3328K(kernel),12800K(rootfs) rootwait quiet rw

$ cat /proc/mtd
dev:    size   erasesize  name
mtd0: 00040000 00010000 "uboot"
mtd1: 00340000 00010000 "kernel"
mtd2: 00c80000 00010000 "rootfs"

$ df -T
Filesystem           Type       1K-blocks      Used Available Use% Mounted on
/dev/root            jffs2          12800     10940      1860  85% /
devtmpfs             devtmpfs       61632         0     61632   0% /dev
tmpfs                tmpfs          61732      6324     55408  10% /tmp
/dev/sda1            vfat        62498880     42304  62456576   0% /mnt/UPAN
```

## u-boot compilation

	apt-get install device-tree-compiler gcc-arm-linux-gnueabihf
	export ARCH=arm
	export CROSS_COMPILE=arm-linux-gnueabihf-
	git clone https://github.com/ARM-software/u-boot.git
	make mx6ull_14x14_evk_defconfig
	make all
	
The device can be seen as "SP Blank 6ULL" when powered by USB-OTG but it is not possible to flash a custom u-boot using imx_usb because it is signed
