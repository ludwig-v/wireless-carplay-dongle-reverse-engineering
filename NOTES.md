## Running on a Mac

Plug the dongle into your Mac. Also connect an iPhone using USB and connect to its Personal Hotspot.

Go to the Mac's System Preferences > Network > iPhone USB and uncheck 'Disable unless needed'.

Connect to the dongle's WiFi and open 192.168.50.2; not everything will work but its a decent start.

## Extract kernel image

Extract kernel image with `dd if=/dev/mtdblock1 of=/tmp/mtd1.bin`

## Build binaries

Checkout the QEMU instructions [here](https://github.com/ludwig-v/wireless-carplay-dongle-reverse-engineering/blob/master/Custom_Firmware/Scripts/Dropbear/NOTES.md)