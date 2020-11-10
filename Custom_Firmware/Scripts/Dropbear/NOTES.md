# Overview

I initially started developing this script using a Raspberry Pi. I ended up switching to QEMU, but I think this could still all be done on the Raspberry Pi.

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

- Created launch `script` (borrowed from [here](https://www.arthurkoziel.com/qemu-ubuntu-20-04/) an [here](https://wiki.qemu.org/index.php/Documentation/Networking#How_to_create_a_virtual_network_device.3F))
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

- Ran launch `script`
```
sudo ./script
```

- Installed Ubuntu 18.04

- Removed from launch `script`:
```
-cdrom ./ubuntu-18.04.5-desktop-amd64.iso
```

- Restarted launch `script`

- Installed OpenSSH Server
```
sudo apt update
sudo apt install openssh-server
sudo systemctl status ssh
sudo ufw allow ssh
```

- SSH into Ubuntu VM (to easily copy and paste)
```
ssh localhost -p 5555
```

- Installed Oh My Zsh terminal (to keep history)
```
sudo apt install zsh
sh -c "$(wget -O- https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"
```

- Install git
```
sudo apt install git
```

- Make `cross` directory
```
mkdir cross
cd cross
```

- Based on info [here](https://developer.technexion.com/mediawiki/index.php/Preparing_a_Toolchain_for_Building_ARM_binaries_on_Linux_hosts#Export_the_environment), download [linaro toolchain](https://releases.linaro.org/components/toolchain/binaries/5.1-2015.08/arm-linux-gnueabihf/) 
```
wget https://releases.linaro.org/components/toolchain/binaries/5.1-2015.08/arm-linux-gnueabihf/
```

- Uncompress linaro toolchain
```
tar xvf gcc-linaro-5.1-2015.08-x86_64_arm-linux-gnueabihf.tar.xz
```

- Add cross platform tools to path
```
export PATH=$PATH:/home/jmillard/cross/opt/gcc-linaro-5.1-2015.08-x86_64_arm-linux-gnueabihf/bin
```

- Cloned repos
```
git clone https://github.com/ludwig-v/wireless-carplay-dongle-reverse-engineering.git
git clone https://github.com/mkj/dropbear.git
```

- Added a symbolic link for `ld-linux.so.3`
```
ln -s /home/jmillard/cross/wireless-carplay-dongle-reverse-engineering/Extracted/28102020/lib/ld-2.20.so /home/jmillard/cross/wireless-carplay-dongle-reverse-engineering/Extracted/28102020/lib/ld-linux.so.3
```

- Install autoconf and make
```
sudo apt install autoconf
sudo apt install make
```

- Build Dropbear
```
cd dropbear
autoreconf
./configure --host arm-linux-gnueabihf --disable-zlib

vi localoptions.h
#define DEBUG_TRACE 1   (creates Dropbear with verbose support)

make -j4 PROGRAMS="dropbear"
```

- Recompile `dropbear` with libraries from dongle
```
arm-linux-gnueabihf-gcc -pie -Wl,-z,now -Wl,-z,relro -o dropbear dbutil.o buffer.o dbhelpers.o dss.o bignum.o signkey.o rsa.o dbrandom.o queue.o atomicio.o compat.o fake-rfc2553.o ltc_prng.o ecc.o ecdsa.o crypto_desc.o curve25519.o ed25519.o dbmalloc.o gensignkey.o gendss.o genrsa.o gened25519.o common-session.o packet.o common-algo.o common-kex.o common-channel.o common-chansession.o termcodes.o loginrec.o tcp-accept.o listener.o process-packet.o dh_groups.o common-runopts.o circbuffer.o list.o netio.o chachapoly.o gcm.o svr-kex.o svr-auth.o sshpty.o svr-authpasswd.o svr-authpubkey.o svr-authpubkeyoptions.o svr-session.o svr-service.o svr-chansession.o svr-runopts.o svr-agentfwd.o svr-main.o svr-x11fwd.o svr-tcpfwd.o svr-authpam.o libtomcrypt/libtomcrypt.a libtommath/libtommath.a  /home/jmillard/cross/wireless-carplay-dongle-reverse-engineering/Extracted/28102020/lib/libc-2.20.so /home/jmillard/cross/wireless-carplay-dongle-reverse-engineering/Extracted/28102020/lib/ld-linux.so.3 /home/jmillard/cross/wireless-carplay-dongle-reverse-engineering/Extracted/28102020/lib/libcrypt-2.20.so /home/jmillard/cross/wireless-carplay-dongle-reverse-engineering/Extracted/28102020/lib/libutil-2.20.so
```

- Verify `dropbear` binary
```
file dropbear
dropbear: ELF 32-bit LSB shared object, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux-armhf.so.3, for GNU/Linux 2.6.32, BuildID[sha1]=f5f9c31ddc0f73cbc454918c5c1478f9271839cd, with debug_info, not stripped
```

- Secure copy `dropbear` binary from `QEMU` to Mac using `Filezilla` (host: `localhost`, port: `5555`)

# Other notes

- Compiling Dropbear without the correct matching `libc` results in `getpwnam` failures such as `Login attempt for nonexistent user`. To debug this, I had to build a static version of `strace`. In the `U2W.sh` script, I would launch Dropbear using:
```
/tmp/strace -f -o /mnt/UPAN/logfile.txt -s 1024 /tmp/dropbear -vFEsg
```

- This also provided helpful logs such as:
```
write(2, "[234] Jan 02 00:01:08 /root must be owned by user or root, and not writable by others\n", 86) = 86
.
.
write(2, "[234] Jan 02 00:01:18 Bad password attempt for 'root' from 192.168.50.100:53685\n", 80) = 80
```

- Compiling Dropbear could probably done in one step. For some reason, the binary always ends up with direct links to `ld-linux-armhf.so.3`. I tried using `-Wl,--dynamic-linker /lib/ld-linux.so.3` with no luck. This is why `U2W.sh` creates the symlink.

- To generate a new public and private key, use the following command (leave the passphrase empty):
```
ssh-keygen -t rsa -C cplay2air -f cplay2air
```

- To build `strace` (on the Raspberry Pi)
```
sudo apt-get install gawk
sudo apt-get install autoconf
git clone https://github.com/strace/strace.git
cd strace
./bootstrap
export LDFLAGS="-L/home/pi/wireless-carplay-dongle-reverse-engineering/Extracted/28102020/lib -static -pthread"
./configure
make
```

- To switch a binaries interrupter
```
sudo apt install patchelf
patchelf --set-interpreter /lib/ld-linux.so.3 dropbear
```

