
![Carlinkit V2](https://i.imgur.com/ZL3dq41.png)

## Carlinkit / Carplay2Air Reverse Engineering

## Hardware

**CPU:** ARMv7 Processor [410fc075] revision 5 (ARMv7), cr=10c53c7d

**Machine model:** Freescale i.MX6 UltraLite 14x14 EVK Board

**ORIGINAL**
| Hardware | Part |
|--|--|
| Flash | Macronix 25L12835F (16MB) |
| CPU | Microchip AT91SAM9260 |
| WIFI chip | Realtek RTL8822BS |

**AUTOKIT**
| Hardware | Part |
|--|--|
| Flash | ? (32MB) |
| CPU | Microchip AT91SAM9260 |
| WIFI chip | Realtek RTL8822BS |

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
