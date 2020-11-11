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
wget https://releases.linaro.org/components/toolchain/binaries/5.1-2015.08/arm-linux-gnueabihf/gcc-linaro-5.1-2015.08-x86_64_arm-linux-gnueabihf.tar.xz
tar xvf gcc-linaro-5.1-2015.08-x86_64_arm-linux-gnueabihf.tar.xz

- Added the linaro toolchain to PATH 
```
export PATH=$PATH:/home/jmillard/toolchain/opt/gcc-linaro-5.1-2015.08-x86_64_arm-linux-gnueabihf/bin
```

- Cloned wireless-carplay-dongle-reverse-engineering repo
```
git clone https://github.com/ludwig-v/wireless-carplay-dongle-reverse-engineering.git
```

- Added a symbolic link for `ld-linux.so.3`
```
ln -s /home/jmillard/toolchain/wireless-carplay-dongle-reverse-engineering/Extracted/28102020/lib/ld-2.20.so /home/jmillard/toolchain/wireless-carplay-dongle-reverse-engineering/Extracted/28102020/lib/ld-linux.so.3
```

- Built `dropbear`
```
sudo apt install autoconf make patchelf
git clone https://github.com/mkj/dropbear.git
cd dropbear
autoreconf
export LDFLAGS="-L/home/jmillard/toolchain/wireless-carplay-dongle-reverse-engineering/Extracted/28102020/lib -l:libc.so -l:libc-2.20.so -l:ld-linux.so.3 -l:libcrypt-2.20.so -l:libutil-2.20.so"
./configure --host arm-linux-gnueabihf --disable-zlib --disable-utmp --disable-wtmp --disable-lastlog

vi localoptions.h
#define DEBUG_TRACE 1   (creates Dropbear with verbose support)

make -j4 PROGRAMS="dropbear"
/home/jmillard/toolchain/opt/gcc-linaro-5.1-2015.08-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-strip dropbear
patchelf --replace-needed ld-linux-armhf.so.3 ld-linux.so.3 dropbear
patchelf --set-interpreter /lib/ld-linux.so.3 dropbear
```

- Verified `dropbear` binary
```
file dropbear
dropbear: ELF 32-bit LSB shared object, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux.so.3, for GNU/Linux 2.6.32, BuildID[sha1]=cf6df6a0252945408a4b45dbdfb8dd51f6e0a5f9, stripped
```

- Built `strace`
```
sudo apt install gawk gcc
git clone https://github.com/strace/strace.git
cd strace
export LDFLAGS="-L/home/jmillard/toolchain/wireless-carplay-dongle-reverse-engineering/Extracted/28102020/lib -l:libc.so -l:libc-2.20.so -l:ld-linux.so.3 -pthread"
./bootstrap
./configure --host arm-linux-gnueabihf
make
/home/jmillard/toolchain/opt/gcc-linaro-5.1-2015.08-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-strip strace
patchelf --replace-needed ld-linux-armhf.so.3 ld-linux.so.3 strace
patchelf --set-interpreter /lib/ld-linux.so.3 strace
```

- Verified `strace` binary
```
file strace
strace: ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux.so.3, for GNU/Linux 2.6.32, BuildID[sha1]=d95eaff6de0cdbe5105d58528d80696b37ac7e90, stripped
```

- Copied `dropbear` and `strace` binaries from `QEMU` using `Filezilla` (host: `localhost`, port: `5555`)

# Other notes

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

