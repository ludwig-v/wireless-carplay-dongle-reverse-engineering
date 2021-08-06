**Work in progress:** reversing new binary (appeared in 2021.03.06) to understand the new encryption mechanism.

The new [ARMimg_maker](https://github.com/ludwig-v/wireless-carplay-dongle-reverse-engineering/blob/master/Reverse/ARMimg_maker) executable contains a [packed executable](https://github.com/ludwig-v/wireless-carplay-dongle-reverse-engineering/tree/master/Reverse/encryptedPackedELF) (at the end of the file) which is encrypted with AES to hide the *.img* decryption logic and password

| Possible packed ELF keys |
| - |
| */etc/bluetooth/temp/addressit\n* |
| */etc/bluetooth/temp/addressme* |

*.tar.gz* is created by decrypting 81920 bytes AES encrypted blocks (from *.img*) using Hardware-Accelerated AES driver (**/dev/hwaes**) : See [STRACE.md](https://github.com/ludwig-v/wireless-carplay-dongle-reverse-engineering/blob/master/Reverse/STRACE.md)

**/dev/hwaes** is similar to **/dev/crypto** which is Hardware-Accelerated AES driver