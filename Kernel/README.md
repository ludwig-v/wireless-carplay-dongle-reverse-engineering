# Carlinkit U2W modern Linux port

This bundle runs the latest Linux LTS available as of July 2026 on the
Carlinkit U2W i.MX6ULL wireless CarPlay adapter. Its purpose is to reproduce
the hardware support and userspace-visible behavior of Carlinkit's Linux 3.14
firmware while retaining a maintained kernel, current fixes, and modern kernel
interfaces.

The working configuration provides:

- the Carlinkit i.MX6ULL board device tree;
- LZMA-compressed ARM `zImage` support for the fixed SPI-NOR layout;
- embedded i.MX SDMA firmware 3.6;
- the vendor RTL8822BS SDIO Wi-Fi driver in WEXT/AP mode;
- the matching Realtek H4/H5 UART Bluetooth and coexistence support;
- compatibility with the legacy BlueZ `hcid` used by U2W firmware;
- the reconstructed iPhone/iAP2, PTP, HNP, and CDC-NCM paths;
- MFi authentication through the vendor I2C/DCP interfaces;
- the vendor `ttyLogFile0` console and matching loadable modules.

Only the Realtek RTL8822BS Wi-Fi/Bluetooth chipset is supported by this build.
Do not use it on BCM, Marvell, RTL8822CS, or other Carlinkit variants.

The Android Auto gadget modules used by U2AC and U2AW products have not been
reconstructed. This bundle targets U2W wireless CarPlay hardware and does not
claim Android Auto support.

## Bundle layout

```text
config/imx6ull-carlinkit-6.18.config
new-files/
patches/
tools/
SHA256SUMS
```

- `config/` contains the production kernel configuration.
- `new-files/` contains board and reconstructed vendor sources absent upstream.
- `patches/series` lists the ordered changes to upstream files.
- `tools/carlinkit-base-pre-rootfs.bin` contains the signed U-Boot, update
  catalogue, and portable environment used by the repacker.
- `tools/update_flash_dump.sh` builds a complete fixed-layout flash image.
- `SHA256SUMS` covers every distributed file except itself.

Several compatibility patches adapt ideas first implemented by the Catplay
project, including the ChipIdea VBUS and role-switch handling, MXS USB PHY
charger-detection behavior, optional direct EROFS-initrd support, BCM device
recognition, and the jitter-entropy startup change. They are adapted to this
kernel and the Carlinkit U2W hardware rather than copied as an independent
platform layer.

The U-Boot image is signed and is a working replacement for newer Carlinkit
bootloaders. It understands the vendor catalogue and performs the required
device-specific DCP conversion on first boot. The bundled kernel region is
blank and is populated by the repacker; the DT in the environment is replaced
with the matching modern board DTB.

The vendor RAM-backed console is retained. The repacker can optionally expose
a physical console on the device's TX1 and RX1 pins at 115200 baud. Use 3.3 V
UART levels; do not connect an RS-232 voltage-level interface directly. On
some units, enabling the physical kernel console changes the behavior of the
proprietary radio startup sufficiently that the Bluetooth name is not
published. Use the RAM-only console mode for normal operation.

## Kernel source and build

Use the pristine kernel.org LTS source:

```text
https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.18.39.tar.xz
```

Apply the bundle to a clean extraction:

```sh
tar -xf linux-6.18.39.tar.xz
cd linux-6.18.39
cp -a /path/to/Kernel/new-files/. .
while read -r patch_name; do
    patch -p1 < "/path/to/Kernel/patches/$patch_name"
done < /path/to/Kernel/patches/series
```

Build with a soft-float `arm-linux-gnueabi-` toolchain. `LOCALVERSION=` keeps
the release name clean. The kernel banner and `/proc/version` deliberately
omit build user, host, compiler distribution, and timestamp information.

```sh
export KERNEL_SRC=$PWD
export KERNEL_BUILD=/path/to/build-carlinkit
mkdir -p "$KERNEL_BUILD"
cp /path/to/Kernel/config/imx6ull-carlinkit-6.18.config \
   "$KERNEL_BUILD/.config"

make O="$KERNEL_BUILD" ARCH=arm \
    CROSS_COMPILE=arm-linux-gnueabi- LOCALVERSION= olddefconfig
make O="$KERNEL_BUILD" ARCH=arm \
    CROSS_COMPILE=arm-linux-gnueabi- LOCALVERSION= \
    -j"$(nproc)" zImage dtbs modules
```

Important outputs:

```text
$KERNEL_BUILD/arch/arm/boot/zImage
$KERNEL_BUILD/arch/arm/boot/dts/nxp/imx/imx6ull-carlinkit.dtb
$KERNEL_BUILD/drivers/net/wireless/rtl8822bs/88x2bs.ko
$KERNEL_BUILD/drivers/bluetooth/rtkbt/rtk_hci_uart.ko
$KERNEL_BUILD/drivers/net/usb/cdc_ncm.ko
$KERNEL_BUILD/drivers/usb/gadget/function/f_ptp.ko
$KERNEL_BUILD/drivers/usb/gadget/legacy/g_iphone.ko
```

LZMA is required because the kernel partition and encrypted staging layout
allow a maximum packaged `zImage` payload of `0x3039c0` bytes. The complete
production configuration fits this limit and boots quickly enough on the
single-core 528 MHz Cortex-A7.

## Important compatibility changes

### Closed descriptors in `select()`

Patch `0026` is required for reliable wireless CarPlay connection and
reconnection. Upstream commit `68514dacf271`, backported to Linux 5.15 as
`145407e54fd1`, made a descriptor closed concurrently by another thread appear
ready to `select()` through `EPOLLNVAL`.

Carlinkit's proprietary multithreaded daemons close and replace sockets during
the Bluetooth-to-Wi-Fi handover. They rely on the older kernel behavior and
can abort the new connection after the changed wakeup. Patch `0026` removes
`EPOLLNVAL` from the `select()` result masks while retaining the rest of the
modern implementation.

### Realtek Wi-Fi and Bluetooth

The RTL8822BS vendor driver is ported to current kernel APIs while retaining
the private `RTL_IOCTL_HOSTAPD` interface required by the U2W hostapd binary.
The driver is built with `-Os` to fit the appliance root filesystem. The
kernel uses the board MAC address and the modern netdev address helper.

The Realtek UART driver retains H4/H5 initialization, firmware loading, and
Bluetooth coexistence. Compatibility changes preserve the setup visibility,
power lifecycle, sysfs address, and old BlueZ interfaces expected by
`bluetoothDaemon` and `hcid`.

The legacy U2W `hcid` passes the original 8-byte `L2CAP_OPTIONS` structure.
The modern kernel otherwise rejects it because the current structure is
larger. Patch `0016` restores bounded copying so SDP PSM `0x0001`, RFCOMM PSM
`0x0003`, pairing, iAP2 discovery, and reconnects work correctly.

### USB, HNP, and CDC-NCM

The reconstructed gadget code provides the iPhone/iAP2, PTP, HNP, and
CDC-NCM sequence required when the adapter changes from USB device to host.
The CDC-NCM compatibility keeps the iPhone NTB sizes, initialization delay,
and malformed-iMAC workaround used by the original firmware.

### Board support

The device tree describes the actual 128 MiB U2W board: SPI-NOR, RTL8822BS
SDIO, Bluetooth UART, USB host/device wiring, MFi I2C, clocks, and pin control.
Always package the matching DTB; the old 3.14 DT does not provide the bindings
required by the modern kernel.

The patch series also preserves the bootloader clock setup, provides UART PIO
fallback when SDMA RX initialization fails, embeds SDMA firmware 3.6, supports
the vendor JFFS2 streams, exposes `ttyLogFile0`, and restores the vendor DCP
interfaces used by firmware installation and MFi authentication.

## Creating a complete U2W flash image

The repacker requires Bash, Python 3, the Carlinkit-compatible Jefferson
JFFS2 extractor, and the host utilities checked at startup. Jefferson and its
Python dependencies must be installed at the path selected by the optional
fifth script argument, or at `/opt/tool/jefferson_carlinkit` by default.

[jefferson_carlinkit repo](https://github.com/ludwig-v/jefferson_carlinkit)

The input dump is used only as a root filesystem donor from U2W firmware.
Nothing before the root filesystem is inherited from that dump. The repacker
installs the signed bootloader base, update catalogue, modern kernel, matching
DTB, and matching modules into a new 16 MiB image.

Root filesystem sanitization is enabled by default. It removes the inherited
UUID, installs a 256-byte zero-filled signature, and removes
Bluetooth bonds, DHCP leases, logs, runtime state, custom radio names, SSH
authorized keys, lockdown pairing state, and AirPlay keychain state. The
existing `/etc/device_serial` is retained because the vendor USB/iAP2 startup
uses it as part of the device identity. Dropbear host keys are retained because
generating replacements on this appliance is slow and depends on additional
userspace tools. `/etc/bluetooth/mybluetooth.tar.gz` is also retained: it is a
vendor Bluetooth bootstrap archive, not merely saved pairing state. The vendor
startup launches Dropbear with blank-password login enabled, so any SSH client
can connect as `root` without an authorized key. Do not expose the adapter's
network interfaces to an untrusted network. Vendor services recreate the
other required hardware-derived identities and keys on the target device.

Both `SANITIZE_ROOTFS=1` and `SANITIZE_ROOTFS=0` extract the donor with
Jefferson and create a new JFFS2 filesystem with `mkfs.jffs2`. Setting the
option to zero preserves the donor's files and identity data, but does not
preserve its original JFFS2 inode numbers, compression choices, node offsets,
or eraseblock packing. Use it only for a private recovery image where
retaining the donor's identity is explicitly intended.

Device activation is not required by this development kernel. Its activation
interface always reports success, allowing the image to be flashed to any
compatible RTL8822BS U2W device without copying another unit's identity or
signature. The proprietary userspace still checks `/etc/uuid_sign` and expects
the original 256-byte file shape, so the sanitizer installs 256 zero bytes.

### Repacker settings

The script is configured through environment variables. `BOOT_CONSOLE_MODE`
controls the kernel command line:

| Value | Kernel consoles and verbosity |
| --- | --- |
| `vendor` | Uses `console=ttyLogFile0` and preserves the donor's `quiet` setting. This is the default. |
| `ram` | Uses `console=ttyLogFile0` and removes `quiet`, retaining normal kernel messages in the RAM-backed log without registering the physical UART as a kernel console. Recommended for normal use. |
| `dual` | Adds `console=ttymxc0,115200 earlyprintk`, retains `ttyLogFile0`, and removes `quiet`. Intended for ordinary serial bring-up. |
| `debug` | Uses both consoles with `earlyprintk loglevel=8 ignore_loglevel` and removes `quiet`. Intended only for short diagnostic runs because printk traffic at 115200 baud can change system timing. |

`ttyLogFile0` is deliberately listed last in dual/debug modes so it remains
the preferred userspace console. Nevertheless, registering `ttymxc0` has been
observed to interfere with Bluetooth-name publication on some adapters. Do
not use dual/debug mode for a production image.

The repacker always ensures the following BusyBox getty entry exists in
`/etc/inittab`, regardless of console or module-installation mode:

```text
ttymxc0::respawn:/sbin/getty -n -l /bin/sh -L ttymxc0 115200 vt100
```

This provides a passwordless root shell on TX1/RX1 at 115200 baud. It is
independent of `BOOT_CONSOLE_MODE`: in `vendor` and `ram` modes the UART is not
a printk console, but getty still opens and configures `ttymxc0` after init
starts. Consequently, RAM-only mode suppresses physical kernel logs but does
not leave UART1 completely unused. Remove or comment this inittab entry in a
custom rootfs if opening UART1 affects radio behavior on a particular unit.

Root filesystem and module settings:

| Variable | Default | Meaning |
| --- | ---: | --- |
| `SANITIZE_ROOTFS` | `1` | Remove the UUID plus pairing, lease, log and remote-access state while retaining `device_serial`, Dropbear host keys and the Bluetooth bootstrap archive; `uuid_sign` becomes 256 zero bytes. |
| `JFFS2_COMPRESSION_MODE` | `priority` | Value passed to `mkfs.jffs2 --compression-mode`. |
| `INSTALL_MODULE_TREE` | `1` | Install the complete matching kernel module tree and run `depmod`. |
| `ROOTFS_MODULES_ONLY` | `0` | Install only the required boot modules. This requires `INSTALL_MODULE_TREE=0` and `INSTALL_DIAGNOSTICS=0`. |
| `PRESERVE_VENDOR_MODULES` | `0` | Keep donor kernel modules instead of replacing them. Use only with an ABI-compatible kernel. |
| `KERNEL_BOOT_MODULES` | built-in list | Whitespace-separated relative module paths replacing the standard Carlinkit boot-module list. |
| `RTL8822_ROOTFS` | unset | Optional userspace overlay associated with an external RTL8822BS import. |
| `INSTALL_HCIATTACH_MARKER_COMPAT` | `0` | Install the compatibility marker expected by the vendor Bluetooth startup scripts. |
| `INSTALL_DIAGNOSTICS` | `1` | Install the diagnostic userspace supplied by the bundle. |

Kernel, DTB and layout settings:

| Variable | Default | Meaning |
| --- | ---: | --- |
| `KERNEL_DTB` | unset | DTB embedded into the signed catalogue environment. Always set it to the DTB built with the kernel. |
| `KERNEL_MTDPARTS_DEVICE` | unset | Replacement for the legacy MTD master in `mtdparts`; use `21e0000.spi` with the supplied DTB. |
| `BASE_PRE_ROOTFS_IMAGE` | bundled base | Alternate signed pre-rootfs image; it must be exactly `0x380000` bytes. |
| `STRIP` | auto-detected | Module-strip executable, normally inferred from `CROSS_COMPILE` or `arm-linux-gnueabi-strip`. |

Typical selections are:

```sh
# Normal image: normal kernel logging in RAM, no physical UART console.
BOOT_CONSOLE_MODE=ram SANITIZE_ROOTFS=1

# Private image retaining donor files and identity data.
BOOT_CONSOLE_MODE=ram SANITIZE_ROOTFS=0

# Temporary physical-UART diagnosis at the normal printk level.
BOOT_CONSOLE_MODE=dual SANITIZE_ROOTFS=0

# Maximum early/boot diagnostics; not recommended for radio testing.
BOOT_CONSOLE_MODE=debug SANITIZE_ROOTFS=0
```

These assignments prefix the complete invocation below.

```sh
BOOT_CONSOLE_MODE=ram \
KERNEL_DTB="$KERNEL_BUILD/arch/arm/boot/dts/nxp/imx/imx6ull-carlinkit.dtb" \
KERNEL_MTDPARTS_DEVICE=21e0000.spi \
INSTALL_DIAGNOSTICS=0 \
INSTALL_MODULE_TREE=0 \
ROOTFS_MODULES_ONLY=1 \
INSTALL_HCIATTACH_MARKER_COMPAT=1 \
SANITIZE_ROOTFS=1 \
./tools/update_flash_dump.sh \
    /path/to/u2w-firmware-dump.bin \
    "$KERNEL_BUILD/arch/arm/boot/zImage" \
    "$KERNEL_BUILD" \
    /path/to/ready-to-flash.bin
```

Do not flash a raw `zImage` through the complete-dump update path. Do not mix
modules from another kernel. Do not interrupt power during the initial
catalogue installation, DCP conversion, and reset cycle.

## Verification

The repacker validates the catalogue, encrypted staging blocks, rebuilt JFFS2
filesystem, module archives, sanitized paths, total image size, and output
layout. The embedded DTB can be checked independently:

```sh
python3 tools/extract_environment_dtb.py ready-to-flash.bin extracted.dtb
cmp extracted.dtb "$KERNEL_BUILD/arch/arm/boot/dts/nxp/imx/imx6ull-carlinkit.dtb"
```

After flashing, verify Wi-Fi association, Bluetooth pairing and repeated
reconnection, SDP/iAP2 discovery, MFi authentication, USB HNP, CDC-NCM
creation, and an actual wireless CarPlay session.
