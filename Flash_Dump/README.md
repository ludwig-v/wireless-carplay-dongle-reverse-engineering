# FLASH/DUMP firmware using Raspberry PI

## interconnection

Connect FLASH chip MX25L12835F to Raspberry PI connector:

| FLASH  | RPI           |
| ------ | ------------- | 
| 1 CS   | 24            |
| 2 DO   | 21            |
| 3 WP   | 1 or 17 (+3V) |
| 4 GND  | 9 or 25 ...   |
| 5 DI   | 19            |
| 6 SCK  | 23            |
| 7 HOLD | 1 or 17 (+3V) |
| 8 VCC  | 1 or 17 (+3V) |

_- Connect it when RPI is disconnected from power supply_

_- During update, do not connect carplay dongle to USB, FLASH chip is powered from RPI_

## ENABLE SPI on RPI

Use `raspi-config` and enable SPI peripheral

## FLASHROM

To access FLASH memory use command `flashrom` wihich is preinstalled in raspios

### Check connection to FLASH memory

```
$ flashrom -p linux_spi:dev=/dev/spidev0.0,spispeed=1000
flashrom v1.2 on Linux 5.10.92-v8+ (aarch64)
flashrom is free software, get the source code at https://flashrom.org

Using clock_gettime for delay loops (clk_id: 1, resolution: 1ns).
Found Macronix flash chip "MX25L12805D" (16384 kB, SPI) on linux_spi.
Found Macronix flash chip "MX25L12835F/MX25L12845E/MX25L12865E" (16384 kB, SPI) on linux_spi.
Multiple flash chip definitions match the detected chip(s): "MX25L12805D", "MX25L12835F/MX25L12845E/MX25L12865E"
Please specify which chip definition to use with the -c <chipname> option.
```

It will found two alternative FLASH memory chips, then we need to use parameter `-c MX25L12835F/MX25L12845E/MX25L12865E`

### Dump FLASH memory into file

```
$ flashrom -p linux_spi:dev=/dev/spidev0.0,spispeed=1000 -c MX25L12835F/MX25L12845E/MX25L12865E -r MX25L12835F_CFW.bin
flashrom v1.2 on Linux 5.10.92-v8+ (aarch64)
flashrom is free software, get the source code at https://flashrom.org

Using clock_gettime for delay loops (clk_id: 1, resolution: 1ns).
Found Macronix flash chip "MX25L12835F/MX25L12845E/MX25L12865E" (16384 kB, SPI) on linux_spi.
Reading flash... done.
```

### Write FLASH memory from file

```
$ flashrom -p linux_spi:dev=/dev/spidev0.0,spispeed=1000 -c MX25L12835F/MX25L12845E/MX25L12865E -w MX25L12835F_CFW_20210830.BIN 
flashrom v1.2 on Linux 5.10.92-v8+ (aarch64)
flashrom is free software, get the source code at https://flashrom.org

Using clock_gettime for delay loops (clk_id: 1, resolution: 1ns).
Found Macronix flash chip "MX25L12835F/MX25L12845E/MX25L12865E" (16384 kB, SPI) on linux_spi.
Reading old flash chip contents... done.
Erasing and writing flash chip... Erase/write done.
Verifying flash... VERIFIED.
```

![IMG_1646](https://user-images.githubusercontent.com/9936533/161439195-3f74987a-7393-4b07-9bd5-c43407691198.jpeg)
