# Overview

This script runs a [Dropbear SSH](https://github.com/mkj/dropbear) server which allows root access to the dongle.

**Run the script only if you really know what you are doing. You can permanently brick your dongle. We accept no responsibility for loss or damage!**

# Usage

- Install the public and private keys
```
tar -xvzf keys.tar.gz
cp cplay2air ~/.ssh
cp cplay2air.pub ~/.ssh
chmod 700 ~/.ssh/cplay2air
chmod 700 ~/.ssh/cplay2air.pub
```

- Copy the following files to the root of a USB drive
```
U2W.sh
dropbear
sftp-server
libz.so.1.2.11
cplay2air.pub
```

- Power on the dongle, wait until the red power led turns on, and insert the USB drive
  - The red power led will turn off to let you know Dropbear is running

- Connect to the WiFi network created by dongle (`GMxx`, `AutoBox-xxxx`, etc.)
  - If prompted for a password, enter `12345678`
  - (If using MacOS, the password would populated via iCloud)

- `SSH` into the dongle
```
ssh -i ~/.ssh/cplay2air root@192.168.50.2
```

- If you receive a host warning, continue by typing `yes`
```
The authenticity of host '192.168.50.2 (192.168.50.2)' can't be established. 
RSA key fingerprint is SHA256:9qg+0BArUwa183yEEwf22sZB+YKh6kzHJj90smg1EYg. 
Are you sure you want to continue connecting (yes/no/[fingerprint])? yes
Warning: Permanently added '192.168.50.2' (RSA) to the list of known hosts.
```

- You should now be logged in to the dongle as `root`
```
[(\t)\u@\w]#
```

- To verify enter the `uname -a` command
```
[(\t)\u@\w]#uname -a
Linux sk_mainboard 3.14.52+g94d07bb #39 SMP PREEMPT Tue Oct 27 01:47:06 CST 2020 armv7l GNU/Linux
```

- `SFTP` into the dongle
```
sftp  -i ~/.ssh/cplay2air root@192.168.50.2
Connected to 192.168.50.2.
sftp>
```

- To verify list the `/usr/lib` folder
```
sftp> ls /usr/lib
/usr/lib/libARMtool.so             /usr/lib/libavcodec.so             /usr/lib/libavcodec.so.56
/usr/lib/libavcodec.so.56.26.100   /usr/lib/libavformat.so            /usr/lib/libavformat.so.56
/usr/lib/libavformat.so.56.25.101  /usr/lib/libavutil.so              /usr/lib/libavutil.so.54
/usr/lib/libavutil.so.54.20.100    /usr/lib/libbluetooth.so           /usr/lib/libbluetooth.so.2
/usr/lib/libbluetooth.so.2.11.2    /usr/lib/libcrypto.so              /usr/lib/libcrypto.so.1.0.0
/usr/lib/libdbus-1.so              /usr/lib/libdbus-1.so.3            /usr/lib/libdbus-1.so.3.2.0
/usr/lib/libdns_sd.so              /usr/lib/libfdk-aac.so             /usr/lib/libfdk-aac.so.1
/usr/lib/libfdk-aac.so.1.0.0       /usr/lib/libswresample.so          /usr/lib/libswresample.so.1
/usr/lib/libswresample.so.1.1.100  /usr/lib/libtinyalsa.so            /usr/lib/libusb-1.0.so
/usr/lib/libusb-1.0.so.0           /usr/lib/libusb-1.0.so.0.1.0       /usr/lib/libxml2.so
/usr/lib/libxml2.so.2              /usr/lib/libxml2.so.2.9.2          /usr/lib/libz.so
```
