
![Carlinkit V2](https://i.imgur.com/ZL3dq41.png)

## Carlinkit / Carplay2Air Reverse Engineering

## Hardware

| Hardware | Part |
|--|--|
| Flash | Macronix 25L12835F (16MB) |
| SoC | Freescale i.MX6 UltraLite |
| CPU | ARM Cortex-A7 (ARMv7) |
| RAM | Micron/SK hynix 1Gb (64x16)
| Wi-Fi/BT | Realtek RTL8822BS |

# Software

**Kernel command line (Carplay2Air):** root=/dev/mtdblock3 rootfstype=jffs2 mtdparts=21e0000.qspi:1M(uboot),5056K(kernel),64K(dtb),10M(rootfs) rootwait rw

## u-boot compilation

	apt-get install device-tree-compiler gcc-arm-linux-gnueabihf
	export ARCH=arm
	export CROSS_COMPILE=arm-linux-gnueabihf-
	git clone https://github.com/ARM-software/u-boot.git
	make mx6ull_14x14_evk_defconfig
	make all
	
The device can be seen as "SP Blank 6ULL" when powered by USB-OTG but it is not possible to flash a custom u-boot using imx_usb because it is signed
