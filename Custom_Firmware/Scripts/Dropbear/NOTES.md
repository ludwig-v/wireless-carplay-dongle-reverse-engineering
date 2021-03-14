# Overview

I initially started developing this script on a Raspberry Pi. I ended up switching to `QEMU`, because I needed a `gcc` that had a `glibc` no higher than `2.20`. I think this could still be done on a Raspberry Pi, but would require an old version of Raspbian.

The following was all performed on MacOS.

# Steps

- Installed `QEMU`

```
brew install qemu
```

- Downloaded Ubuntu 18.04 ISO

- Created a blank disk image

```
qemu-img create -f qcow2 ubuntu-desktop-18.04.qcow2 10G
```

- Created `launch.sh` script (borrowed from [here](https://www.arthurkoziel.com/qemu-ubuntu-20-04/) an [here](https://wiki.qemu.org/index.php/Documentation/Networking#How_to_create_a_virtual_network_device.3F))

```
qemu-system-x86_64 \
    -machine type=q35,accel=hvf \
    -cpu Nehalem \
    -smp 2 \
    -hda ubuntu-desktop-18.04.qcow2 \
    -m 4G \
    -vga virtio \
    -usb \
    -device usb-tablet \
    -display default,show-cursor=on \
    -device e1000,netdev=net0 \
    -cdrom ./ubuntu-18.04.5-desktop-amd64.iso \
    -netdev user,id=net0,hostfwd=tcp::5555-:22
```

- Started `QEMU` 

```
sudo ./launch.sh
```

- Installed Ubuntu 18.04

- Removed `cdrom` entry from `launch.sh`

```
-cdrom ./ubuntu-18.04.5-desktop-amd64.iso
```

- Restarted `QEMU`

```
sudo ./launch.sh
```

- Installed OpenSSH Server

```
sudo apt update
sudo apt install openssh-server
sudo systemctl status ssh
sudo ufw allow ssh
```

- SSH into Ubuntu 18.04 (to easily copy and paste)

```
ssh localhost -p 5555
```

- Installed `Oh My Zsh` (to keep history)

```
sudo apt install zsh
sh -c "$(wget -O- https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"
```

- Installed git

```
sudo apt install git
```

- Installed a linaro toolchain based on information [here](https://developer.technexion.com/mediawiki/index.php/Preparing_a_Toolchain_for_Building_ARM_binaries_on_Linux_hosts#Export_the_environment) and [here](https://releases.linaro.org/components/toolchain/binaries/5.1-2015.08/arm-linux-gnueabihf/) 

```
mkdir toolchain
cd toolchain
wget https://releases.linaro.org/components/toolchain/binaries/5.1-2015.08/arm-linux-gnueabi/gcc-linaro-5.1-2015.08-x86_64_arm-linux-gnueabi.tar.xz
tar xvf gcc-linaro-5.1-2015.08-x86_64_arm-linux-gnueabi.tar.xz
```

- Added the linaro toolchain to PATH 

```
export PATH=/home/jmillard/toolchain/gcc-linaro-5.1-2015.08-x86_64_arm-linux-gnueabi/bin:$PATH
```

 Cloned wireless-carplay-dongle-reverse-engineering repo

```
git clone https://github.com/ludwig-v/wireless-carplay-dongle-reverse-engineering.git
```

- Built `dropbear`

```
sudo apt install autoconf make 
git clone https://github.com/mkj/dropbear.git
cd dropbear
autoreconf
./configure --host arm-linux-gnueabi --disable-zlib --disable-utmp --disable-wtmp --disable-lastlog
echo "#define DEBUG_TRACE 1" > localoptions.h
make -j4 PROGRAMS="dropbear"
arm-linux-gnueabi-strip dropbear
```

- Verified `dropbear` binary

```
file dropbear
dropbear: ELF 32-bit LSB shared object, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux.so.3, for GNU/Linux 2.6.32, BuildID[sha1]=ebd40be6687ff903bdfc32464125d3c157d8fb31, stripped
```

- Built `libz.so.1.2.11`

```
cd ..
wget https://zlib.net/zlib-1.2.11.tar.gz
tar xvf zlib-1.2.11.tar.gz
cd zlib-1.2.11
CHOST=arm-linux-gnueabi ./configure
make
arm-linux-gnueabi-strip libz.so.1.2.11
```

- Verified `libz.so.1.2.11` binary

```
file libz.so.1.2.11
libz.so.1.2.11: ELF 32-bit LSB shared object, ARM, EABI5 version 1 (SYSV), dynamically linked, BuildID[sha1]=a81da99f011c0a433ad040413b932da72cc7b6c9, stripped
```

- Build `sftp-server`

```
cd ..
wget https://cdn.openbsd.org/pub/OpenBSD/OpenSSH/portable/openssh-8.5p1.tar.gz
tar xvf openssh-8.5p1.tar.gz
openssh-8.5p1
./configure --host arm-linux-gnueabi --with-zlib=../zlib-1.2.11 --without-openssl
make sftp-server
arm-linux-gnueabi-strip sftp-server
```

- Verified `sftp-server` binary

```
file sftp-server
sftp-server: ELF 32-bit LSB shared object, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux.so.3, for GNU/Linux 2.6.32, BuildID[sha1]=03452d94a538d114fca03372afe426b5d79fb07c, stripped
```

- Built `strace`

```
cd ..
sudo apt install gawk gcc
git clone https://github.com/strace/strace.git
cd strace
./bootstrap
./configure --host arm-linux-gnueabi
make
arm-linux-gnueabi-strip src/strace
```

- Verified `strace` binary

```
file src/strace
src/strace: ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux.so.3, for GNU/Linux 2.6.32, BuildID[sha1]=26ace7e9b6217c60ed0aa22b69fd8c251edcf024, stripped
```

- Copied `dropbear`, `libz.so.1.2.11`, `sftp-server`, and `strace` binaries from `QEMU` using `Filezilla` (host: `localhost`, port: `5555`)

# Other notes

- The first release of `dropbear` and `strace` was compiled using the `hard-float ABI` version of the linaro toolchain. I could not get `libz` as a shared object to run. After running `readelf -h` on other libraries on the device, I noticed they were all `soft-float ABI`. After switching toolchains, you no longer have to run `patchelf` to switch the interpreter.

- Compiling Dropbear without the correct matching `libc` results in `getpwnam` failures such as `Login attempt for nonexistent user`. To debug this, I had to use `strace`. In the `U2W.sh` script, I would launch Dropbear using:

```
/tmp/strace -f -o /mnt/UPAN/logfile.txt -s 1024 /tmp/dropbear -vFEsg
```

- This also provided helpful logs such as

```
write(2, "[234] Jan 02 00:01:08 /root must be owned by user or root, and not writable by others\n", 86) = 86
.
.
write(2, "[234] Jan 02 00:01:18 Bad password attempt for 'root' from 192.168.50.100:53685\n", 80) = 80
```

- To generate a new public and private key, use the following command (leave the passphrase empty)

```
ssh-keygen -t rsa -C cplay2air -f cplay2air
```

