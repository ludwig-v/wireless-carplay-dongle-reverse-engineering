# FAQ

## How can I run this on a Raspberry Pi/DIY hardware?

This repo is only concerned with reverse-engineering USB dongles
from Carlinkit and CPLAY2Air. See [this discussion](https://github.com/ludwig-v/wireless-carplay-dongle-reverse-engineering/discussions/35) for more details.

## How do I use the custom firmware?

Same way you'd update to official firmware over USB; [check here](https://github.com/ludwig-v/wireless-carplay-dongle-reverse-engineering/blob/master/Firmware/README.md) for more details.

## How do I build binaries?

Checkout the QEMU instructions [here](https://github.com/ludwig-v/wireless-carplay-dongle-reverse-engineering/blob/master/Custom_Firmware/Scripts/Dropbear/NOTES.md)

## How do I run the dongle without the car?

1. Plug the dongle into your Mac. 
2. Connect an iPhone to your Mac using USB and connect to its Personal Hotspot.
3. Go to the Mac's `System Preferences > Network > iPhone USB` and uncheck `Disable unless needed`.
4. Connect to the dongle's WiFi and open 192.168.50.2.