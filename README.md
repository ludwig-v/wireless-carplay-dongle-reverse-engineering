

![Carlinkit V2](https://i.imgur.com/ZL3dq41.png)

## Carlinkit / Carplay2Air Reverse Engineering

## Hardware

| Hardware | Part |
|--|--|
| Flash | Macronix 25L12835F (16MB) |
| SoC | Freescale i.MX6 UltraLite |
| CPU | ARM Cortex-A7 (ARMv7) |
| RAM | Micron/SK hynix 1Gb (64x16)
| Wi-Fi/BT | RTL8822BS (Realtek) or Fn-Link L287B-SR (Marvell) or LGX4358 (Broadcom) or LGX8354S (Broadcom) |

## Hardware differences

Hardware base remains almost exactly the same since the beginning, only device case / design changed (Thermal pads on V4 / Holes for air flow on V5)

Wifi chip only seems to be changed depending on the stock of the manufacturer. Whatever Wifi chip is being used, is connected over SDIO Protocol (High Speed 4-Bit SD) at 50Mhz, for a maximum speed of 25MB/s (200 Mbps)

Carlinkit created its own segmentation mainly for marketing using different softwares:
- 1.0 / 2.0 / 3.0 = U2W : Only Carplay Wired OEM for Wireless Carplay
- 4.0 = U2AW : Only Carplay Wired OEM for Wireless Carplay + Wireless Android Auto
- 5.0 = U2AC : Carplay Wired OEM or Android Auto Wired OEM for both Wireless Carplay and Wireless Android Auto

![Box Types](https://github.com/ludwig-v/wireless-carplay-dongle-reverse-engineering/blob/master/Pictures/BoxTypes.png?raw=true)

# Software

It is possible to switch from one to another by rewriting the flash chip using a programmer (ASProgrammer, XGEcu devices, Raspberry Pi) 
Tested so far:
 - 2.0 to U2AW ☑️ *(Only with 2022.06.09.1652 for Realtek Wifi)*
 - 3.0 to U2AW ✅
 - 2.0 to U2AC ❌ *(Not booting)*
 - 3.0 to U2AC ❓ *(Might work)*
 - 4.0 to U2W ✅
 - 4.0 to U2AW ❌ *(Not booting)*
 - 5.0 to U2W ✅
 - 5.0 to U2AW ❌ *(Not booting)*

Please note that **Carlinkit controls devices activation** (via /etc/uuid_sign) so it may not work in the future, **save your original flash**.

## Filesystem

Custom JFFS2 implementation has been made by Carlinkit, they use custom compression algorithm 0x1**6** which seems to be based on Zlib (JFFS2_COMPR_ZLIB = 0x0**6**)

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

No reverse engineering has been done on this yet because JFFS2 is a compiled build-in module of Carlinkit kernel.
Due to this we can't extract files from flash dumps nor editing them.

Compression algorithm could be implemented in jefferson (when known) like this: https://github.com/onekey-sec/jefferson/pull/19/commits/c817964c9d6484c3c7c8c3a167cda98816d342b2

## u-boot compilation

	apt-get install device-tree-compiler gcc-arm-linux-gnueabihf
	export ARCH=arm
	export CROSS_COMPILE=arm-linux-gnueabihf-
	git clone https://github.com/ARM-software/u-boot.git
	make mx6ull_14x14_evk_defconfig
	make all
	
The device can be seen as "SP Blank 6ULL" when powered by USB-OTG but it is not possible to flash a custom u-boot using imx_usb because it is signed

## Links to interesting repos

https://github.com/segfly/carlinkit-modding

https://github.com/Henkru/cplay2air-wifi-passphrase-patch

https://github.com/Quikeramos1/Unbrik-Carlinkit-V3
