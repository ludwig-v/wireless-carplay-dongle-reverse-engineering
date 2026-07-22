#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat >&2 <<'EOF'
Usage:
  update_flash_dump.sh <input-dump.bin> <zImage> <kernel-build-dir> \
      <output-dump.bin> [jefferson-carlinkit-dir]

The input dump is used only as a JFFS2 rootfs donor. Everything before the
rootfs comes from the bundled known-good U2W_2021.03.06.1343 base prefix,
including 2021 U-Boot, the update catalogue, and its valid LZO payload
containing the vendor i.MX6UL DT prepared for Carlinkit's Linux 3.14 kernel.
The repacker replaces that DT with KERNEL_DTB. The output also contains the
new kernel remainder and three portable encrypted kernel staging blocks. It
fills the installed environment with 0xFF so boot_init() enters the catalogue
update path on first boot. That path installs the portable environment and
converts the staged blocks to the i.MX6ULL DCP device-unique representation.
EOF
    exit 1
}

[[ $# -ge 4 && $# -le 5 ]] || usage

INPUT_DUMP=$1
ZIMAGE=$2
KERNEL_BUILD=$(realpath -- "$3")
OUTPUT_DUMP=$4
JEFFERSON_DIR=${5:-/opt/tool/jefferson_carlinkit}
PRESERVE_VENDOR_MODULES=${PRESERVE_VENDOR_MODULES:-0}
RTL8822_ROOTFS=${RTL8822_ROOTFS:-}
INSTALL_DIAGNOSTICS=${INSTALL_DIAGNOSTICS:-1}
JFFS2_COMPRESSION_MODE=${JFFS2_COMPRESSION_MODE:-priority}
INSTALL_MODULE_TREE=${INSTALL_MODULE_TREE:-1}
BOOT_CONSOLE_MODE=${BOOT_CONSOLE_MODE:-vendor}
ROOTFS_MODULES_ONLY=${ROOTFS_MODULES_ONLY:-0}
INSTALL_HCIATTACH_MARKER_COMPAT=${INSTALL_HCIATTACH_MARKER_COMPAT:-0}
SANITIZE_ROOTFS=${SANITIZE_ROOTFS:-1}
KERNEL_DTB=${KERNEL_DTB:-}
KERNEL_MTDPARTS_DEVICE=${KERNEL_MTDPARTS_DEVICE:-}
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOTFS_OVERLAY="$SCRIPT_DIR/../new-files/rootfs"
BASE_PRE_ROOTFS_IMAGE=${BASE_PRE_ROOTFS_IMAGE:-"$SCRIPT_DIR/carlinkit-base-pre-rootfs.bin"}

FLASH_SIZE=$((0x1000000))
KERNEL_START=$((0x40000))
KERNEL_SIZE=$((0x3039c0))
KERNEL_PREFIX_SIZE=$((0x30000))
KERNEL_REMAINDER_START=$((KERNEL_START + KERNEL_PREFIX_SIZE))
KERNEL_REMAINDER_SIZE=$((KERNEL_SIZE - KERNEL_PREFIX_SIZE))
STAGING_START=$((0x3439c0))
BLOCK_SIZE=$((0x10000))
ROOTFS_START=$((0x380000))
ROOTFS_SIZE=$((0xc80000))
ENVIRONMENT_START=$((0x30000))
CATALOGUE_OFFSET=$((0x800))
CATALOGUE_SCAN_SIZE=$((0x19a))
CATALOGUE_ENTRY_SIZE=$((0x29))
CATALOGUE_OFFSET_FIELD=$((0x21))
CATALOGUE_SIZE_FIELD=$((0x25))

KEY=$(printf '%s' 'https://apple.co' | xxd -p -c 256)
IV=$(printf '%s' 'm/kb/HT208050   ' | xxd -p -c 256)

JEFFERSON_BIN="$JEFFERSON_DIR/.deps/bin/jefferson"
JEFFERSON_PYTHONPATH="$JEFFERSON_DIR/.deps:$JEFFERSON_DIR"

MODULE_PATHS=(
    drivers/net/usb/cdc_ncm.ko
    drivers/usb/gadget/function/f_ptp.ko
    drivers/usb/gadget/legacy/g_iphone.ko
    drivers/bluetooth/rtkbt/rtk_hci_uart.ko
    drivers/net/wireless/rtl8822bs/88x2bs.ko
)

# Newer bring-up kernels may not yet provide every reconstructed Carlinkit
# module.  Keep the complete 3.14 list as the default, but allow an explicit
# whitespace-separated list of ABI-matching boot modules for test images.
if [[ -n "${KERNEL_BOOT_MODULES:-}" ]]; then
    read -r -a MODULE_PATHS <<< "$KERNEL_BOOT_MODULES"
fi

for command in cp dd depmod grep md5sum mkfs.jffs2 mktemp od openssl python3 \
               realpath sed sha256sum stat tar touch tr xxd; do
    command -v "$command" >/dev/null || {
        echo "Error: required command not found: $command" >&2
        exit 1
    }
done

[[ -f "$INPUT_DUMP" ]] || { echo "Error: dump not found: $INPUT_DUMP" >&2; exit 1; }
[[ -f "$BASE_PRE_ROOTFS_IMAGE" ]] || {
    echo "Error: known-good base pre-rootfs image not found: $BASE_PRE_ROOTFS_IMAGE" >&2
    exit 1
}
case "$BOOT_CONSOLE_MODE" in
    vendor|ram|dual|debug) ;;
    *) echo "Error: BOOT_CONSOLE_MODE must be vendor, ram, dual, or debug" >&2; exit 1 ;;
esac
if [[ -n "$KERNEL_MTDPARTS_DEVICE" &&
      ! "$KERNEL_MTDPARTS_DEVICE" =~ ^[A-Za-z0-9._-]+$ ]]; then
    echo "Error: KERNEL_MTDPARTS_DEVICE contains unsupported characters" >&2
    exit 1
fi
[[ -f "$ZIMAGE" ]] || { echo "Error: zImage not found: $ZIMAGE" >&2; exit 1; }
if [[ -n "$KERNEL_DTB" ]]; then
    [[ -f "$KERNEL_DTB" ]] || {
        echo "Error: replacement DTB not found: $KERNEL_DTB" >&2
        exit 1
    }
fi
if [[ $(realpath -m -- "$INPUT_DUMP") == $(realpath -m -- "$OUTPUT_DUMP") ]]; then
    echo "Error: output dump must be different from the input dump" >&2
    exit 1
fi
[[ -x "$JEFFERSON_BIN" ]] || {
    echo "Error: Jefferson Carlinkit is not installed at $JEFFERSON_DIR" >&2
    echo "Install its dependencies with:" >&2
    echo "  python3 -m pip install --target '$JEFFERSON_DIR/.deps' '$JEFFERSON_DIR'" >&2
    exit 1
}
if (( INSTALL_DIAGNOSTICS )) && [[ ! -x "$ROOTFS_OVERLAY/usr/bin/strace" ]]; then
    echo "Error: executable rootfs overlay is missing: /usr/bin/strace" >&2
    exit 1
fi
if (( ROOTFS_MODULES_ONLY )); then
    (( ! INSTALL_DIAGNOSTICS )) || {
        echo "Error: ROOTFS_MODULES_ONLY=1 requires INSTALL_DIAGNOSTICS=0" >&2
        exit 1
    }
    (( ! INSTALL_MODULE_TREE )) || {
        echo "Error: ROOTFS_MODULES_ONLY=1 requires INSTALL_MODULE_TREE=0" >&2
        exit 1
    }
    [[ -z "$RTL8822_ROOTFS" ]] || {
        echo "Error: ROOTFS_MODULES_ONLY=1 does not permit a userspace overlay" >&2
        exit 1
    }
fi
if [[ $(stat -c '%s' "$INPUT_DUMP") -ne $FLASH_SIZE ]]; then
    printf 'Error: input dump must be exactly 0x%X bytes\n' "$FLASH_SIZE" >&2
    exit 1
fi
if [[ $(stat -c '%s' "$BASE_PRE_ROOTFS_IMAGE") -ne $ROOTFS_START ]]; then
    printf 'Error: base pre-rootfs image must be exactly 0x%X bytes\n' \
        "$ROOTFS_START" >&2
    exit 1
fi

zimage_size=$(stat -c '%s' "$ZIMAGE")
if (( zimage_size > KERNEL_SIZE )); then
    printf 'Error: zImage is 0x%X bytes; bootloader reads only 0x%X bytes\n' \
        "$zimage_size" "$KERNEL_SIZE" >&2
    exit 1
fi

magic=$(dd if="$ZIMAGE" bs=1 skip=$((0x24)) count=4 status=none | xxd -p)
[[ "$magic" == "18286f01" ]] || {
    echo "Error: input does not contain ARM zImage magic at offset 0x24" >&2
    exit 1
}

rootfs_magic=$(dd if="$INPUT_DUMP" bs=1 skip="$ROOTFS_START" count=2 status=none | xxd -p)
[[ "$rootfs_magic" == "8519" ]] || {
    printf 'Error: JFFS2 magic was not found at flash offset 0x%X\n' "$ROOTFS_START" >&2
    exit 1
}

if (( ! PRESERVE_VENDOR_MODULES )); then
    for module in "${MODULE_PATHS[@]}"; do
        [[ -f "$KERNEL_BUILD/$module" ]] || {
            echo "Error: required module is missing: $KERNEL_BUILD/$module" >&2
            exit 1
        }
    done
fi

STRIP=${STRIP:-}
if [[ -z "$STRIP" ]]; then
    if [[ -n "${CROSS_COMPILE:-}" ]] &&
       command -v "${CROSS_COMPILE}strip" >/dev/null; then
        STRIP="${CROSS_COMPILE}strip"
    elif command -v arm-linux-gnueabi-strip >/dev/null; then
        STRIP=arm-linux-gnueabi-strip
    else
        echo "Error: ARM strip tool not found; set STRIP or CROSS_COMPILE" >&2
        exit 1
    fi
fi

WORKDIR=$(mktemp -d)
trap 'rm -rf "$WORKDIR"' EXIT

# Normalize every supplied dump to the known-good base layout. No byte before
# ROOTFS_START is inherited from the input dump. Its only contribution is the
# JFFS2 partition, which is extracted and rebuilt below.
cp --reflink=auto "$INPUT_DUMP" "$WORKDIR/input.base-prefix.bin"
dd if="$BASE_PRE_ROOTFS_IMAGE" of="$WORKDIR/input.base-prefix.bin" \
   bs=1 count="$ROOTFS_START" conv=notrunc status=none
SOURCE_DUMP="$WORKDIR/input.base-prefix.bin"

read_le32() {
    local file=$1 offset=$2 b0 b1 b2 b3
    read -r b0 b1 b2 b3 < <(od -An -v -tu1 -N4 -j "$offset" "$file")
    printf '%u\n' "$((b0 | (b1 << 8) | (b2 << 16) | (b3 << 24)))"
}

echo "[1/7] Validating the portable environment update catalogue"
active_entry=-1
for ((relative = 0; relative < CATALOGUE_SCAN_SIZE; relative += CATALOGUE_ENTRY_SIZE)); do
    marker=$(od -An -v -tu1 -N1 -j $((CATALOGUE_OFFSET + relative)) \
        "$SOURCE_DUMP" | tr -d '[:space:]')
    if [[ "$marker" == "63" ]]; then
        active_entry=$((CATALOGUE_OFFSET + relative))
        break
    fi
done
(( active_entry >= 0 )) || {
    echo "Error: no active '?' environment update entry was found" >&2
    exit 1
}

environment_update_offset=$(read_le32 "$SOURCE_DUMP" \
    $((active_entry + CATALOGUE_OFFSET_FIELD)))
environment_update_size=$(read_le32 "$SOURCE_DUMP" \
    $((active_entry + CATALOGUE_SIZE_FIELD)))
if (( environment_update_size <= 0 || environment_update_size % 16 != 0 ||
      environment_update_offset + environment_update_size > FLASH_SIZE )); then
    echo "Error: catalogue environment update has an invalid range" >&2
    exit 1
fi

dd if="$SOURCE_DUMP" of="$WORKDIR/environment-update.encrypted" \
   bs=1 skip="$environment_update_offset" count="$environment_update_size" status=none
openssl enc -d -aes-128-cbc -K "$KEY" -iv "$IV" -nopad \
    -in "$WORKDIR/environment-update.encrypted" \
    -out "$WORKDIR/environment-update.decrypted"
PYTHONPATH="$JEFFERSON_PYTHONPATH" python3 - \
    "$WORKDIR/environment-update.decrypted" \
    "$WORKDIR/environment-update.serial.decrypted" \
    "$environment_update_size" "$BOOT_CONSOLE_MODE" "$KERNEL_DTB" \
    "$KERNEL_MTDPARTS_DEVICE" <<'PY'
import sys
import zlib
import lzo
from pathlib import Path
from lzallright import LZOCompressor

encrypted_update = Path(sys.argv[1]).read_bytes()
output = Path(sys.argv[2])
allocation = int(sys.argv[3])
console_mode = sys.argv[4]
replacement_dtb_path = sys.argv[5]
mtdparts_device = sys.argv[6]
compressed_size = int.from_bytes(encrypted_update[:4], "little")
if compressed_size <= 0 or compressed_size > len(encrypted_update) - 4:
    raise SystemExit("invalid environment LZO length")
environment = bytearray(LZOCompressor.decompress(
    encrypted_update[4 : 4 + compressed_size]
))
if len(environment) != 0x10000:
    raise SystemExit("environment update does not decompress to 0x10000 bytes")
stored_crc = int.from_bytes(environment[:4], "little")
actual_crc = zlib.crc32(environment[4:0x800]) & 0xFFFFFFFF
if stored_crc != actual_crc:
    raise SystemExit("environment update CRC32 mismatch")
if environment[0x800:0x804] != bytes.fromhex("d00dfeed"):
    raise SystemExit("environment update does not contain its DTB at 0x800")
old_dtb_size = int.from_bytes(environment[0x804:0x808], "big")
if old_dtb_size <= 0x28 or old_dtb_size > len(environment) - 0x800:
    raise SystemExit("environment update contains an invalid DTB size")

if replacement_dtb_path:
    dtb = Path(replacement_dtb_path).read_bytes()
    if dtb[:4] != bytes.fromhex("d00dfeed"):
        raise SystemExit("replacement file does not contain DTB magic")
    dtb_size = int.from_bytes(dtb[4:8], "big")
    if dtb_size <= 0x28 or dtb_size > len(dtb):
        raise SystemExit("replacement DTB has an invalid totalsize")
    dtb = dtb[:dtb_size]
    if len(dtb) > len(environment) - 0x800:
        raise SystemExit("replacement DTB exceeds its environment region")
    environment[0x800:] = b"\0" * (len(environment) - 0x800)
    environment[0x800:0x800 + len(dtb)] = dtb
    print(f"  replacing environment DTB: {old_dtb_size:#x} -> {len(dtb):#x}")

# Preserve the RAM-backed vendor log console exactly.  fakeiOSDevice's launcher
# detects the literal ttyLogFile token in /proc/cmdline and relies on inherited
# stdout/stderr.  A separate ttymxc0 getty remains available after boot without
# registering that UART as a kernel console or sending printk output to it.
# The NUL-separated U-Boot environment occupies bytes 4..0x7ff.
old = b"console=ttyLogFile0"
if console_mode == "vendor":
    new = b"console=ttyLogFile0"
elif console_mode == "ram":
    new = b"console=ttyLogFile0"
elif console_mode == "dual":
    new = b"console=ttymxc0,115200 earlyprintk console=ttyLogFile0"
else:
    new = (b"console=ttymxc0,115200 earlyprintk loglevel=8 "
           b"ignore_loglevel console=ttyLogFile0")
env_data = bytes(environment[4:0x800])
matches = env_data.count(old)
if matches != 1:
    raise SystemExit(f"expected one vendor console setting, found {matches}")
entries = []
mtdparts_matches = 0
for entry in env_data.split(b"\0"):
    if not entry:
        break
    entry = entry.replace(old, new)
    if console_mode in ("ram", "dual", "debug"):
        # Recovery images must show the normal kernel boot log.  Debug mode
        # additionally overrides the kernel's normal console log level.
        entry = entry.replace(b" rootwait quiet rw", b" rootwait rw")
    if mtdparts_device:
        old_mtdparts = b"mtdparts=21e0000.qspi:"
        new_mtdparts = b"mtdparts=" + mtdparts_device.encode("ascii") + b":"
        if old_mtdparts in entry:
            entry = entry.replace(old_mtdparts, new_mtdparts)
            mtdparts_matches += 1
            print(f"  replacing MTD master in bootargs: 21e0000.qspi -> {mtdparts_device}")
        elif new_mtdparts in entry:
            mtdparts_matches += 1
            print(f"  MTD master in bootargs is already {mtdparts_device}")
    entries.append(entry)
if mtdparts_device and mtdparts_matches != 1:
    raise SystemExit(
        f"expected one legacy MTD master in U-Boot environment, found {mtdparts_matches}"
    )
env_data = b"\0".join(entries) + b"\0\0"
if len(env_data) > 0x7fc:
    raise SystemExit("modified U-Boot environment exceeds 0x800-byte region")
environment[4:0x800] = env_data.ljust(0x7fc, b"\0")
environment[:4] = (zlib.crc32(environment[4:0x800]) & 0xFFFFFFFF).to_bytes(4, "little")

compressed = min(
    (lzo.compress(bytes(environment), level, False) for level in (1, 9)),
    key=len,
)
payload = len(compressed).to_bytes(4, "little") + compressed
if len(payload) > allocation:
    raise SystemExit("serial-console environment exceeds catalogue allocation")
output.write_bytes(payload.ljust(allocation, b"\0"))
PY
openssl enc -aes-128-cbc -K "$KEY" -iv "$IV" -nopad \
    -in "$WORKDIR/environment-update.serial.decrypted" \
    -out "$WORKDIR/environment-update.serial.encrypted"
printf '  catalogue entry 0x%X: update 0x%X+0x%X is valid\n' \
    "$active_entry" "$environment_update_offset" "$environment_update_size"

echo "[2/7] Preparing fixed-size kernel image"
cp "$ZIMAGE" "$WORKDIR/zImage.padded"
truncate -s "$KERNEL_SIZE" "$WORKDIR/zImage.padded"

echo "[3/7] Creating portable encrypted kernel staging blocks"
for ((index = 0; index < 3; index++)); do
    dd if="$WORKDIR/zImage.padded" \
       of="$WORKDIR/kernel-$index.plain" \
       bs=1 skip=$((index * BLOCK_SIZE)) count="$BLOCK_SIZE" status=none
    openssl enc -aes-128-cbc -K "$KEY" -iv "$IV" -nopad \
        -in "$WORKDIR/kernel-$index.plain" \
        -out "$WORKDIR/kernel-$index.encrypted"
    openssl enc -d -aes-128-cbc -K "$KEY" -iv "$IV" -nopad \
        -in "$WORKDIR/kernel-$index.encrypted" \
        -out "$WORKDIR/kernel-$index.check"
    cmp "$WORKDIR/kernel-$index.plain" "$WORKDIR/kernel-$index.check"
done

echo "[4/7] Extracting Carlinkit JFFS2 rootfs"
dd if="$SOURCE_DUMP" of="$WORKDIR/rootfs.original.jffs2" \
   bs=1 skip="$ROOTFS_START" count="$ROOTFS_SIZE" status=none
PYTHONPATH="$JEFFERSON_PYTHONPATH" "$JEFFERSON_BIN" \
    "$WORKDIR/rootfs.original.jffs2" -d "$WORKDIR/rootfs" \
    > "$WORKDIR/jefferson-extract.log"

# A firmware dump is a filesystem snapshot and can contain the previous
# owner's identity, pairing databases, leases, logs, and remote-access keys.
# Production images are sanitized by default. Hardware-derived identities and
# keys are recreated by the vendor services on the target device.
if (( SANITIZE_ROOTFS )); then
    rm -f \
        "$WORKDIR/rootfs/etc/uuid" \
        "$WORKDIR/rootfs/etc/uuid_sign" \
        "$WORKDIR/rootfs/etc/.custom_bluetooth_name" \
        "$WORKDIR/rootfs/etc/.custom_wifi_name" \
        "$WORKDIR/rootfs/etc/log_file" \
        "$WORKDIR/rootfs/var/lib/dhcp6s_duid" \
        "$WORKDIR/rootfs/var/lib/udhcpd.leases"
    rm -rf \
        "$WORKDIR/rootfs/etc/bluetooth/temp" \
        "$WORKDIR/rootfs/root/.ssh" \
        "$WORKDIR/rootfs/var/lib/lockdown" \
        "$WORKDIR/rootfs/var/log" \
        "$WORKDIR/rootfs/var/run" \
        "$WORKDIR/rootfs/var/tmp" \
        "$WORKDIR/rootfs/tmp" \
        "$WORKDIR/rootfs/Library/Keychains"
    mkdir -p \
        "$WORKDIR/rootfs/var/log" \
        "$WORKDIR/rootfs/var/run" \
        "$WORKDIR/rootfs/var/tmp" \
        "$WORKDIR/rootfs/tmp" \
        "$WORKDIR/rootfs/Library/Keychains"
    printf 'AutoKit\0' > "$WORKDIR/rootfs/etc/bluetooth_name"
    printf 'AutoBox\0' > "$WORKDIR/rootfs/etc/wifi_name"
    # The development DCP driver accepts activation unconditionally.  Keep the
    # proprietary userspace's expected 256-byte signature-file shape while
    # avoiding reuse of the donor device's signature.
    dd if=/dev/zero of="$WORKDIR/rootfs/etc/uuid_sign" \
        bs=256 count=1 status=none
fi
# Replace chipset-dependent userspace as one coherent payload.  This installs
# the Realtek-enabled hostapd/rtk_hciattach, RTL firmware and hostapd.conf.
# The bundled vendor module archive is replaced with this build's modules in
# the packaging step below.
if [[ -n "$RTL8822_ROOTFS" ]]; then
    [[ -f "$RTL8822_ROOTFS" ]] || {
        echo "Error: RTL8822 rootfs payload was not found: $RTL8822_ROOTFS" >&2
        exit 1
    }
    tar --no-same-owner -xzf "$RTL8822_ROOTFS" -C "$WORKDIR/rootfs"
fi

[[ -f "$WORKDIR/rootfs/script/ko.tar.gz" ]] || {
    echo "Error: /script/ko.tar.gz was not found in the rootfs" >&2
    exit 1
}
[[ -f "$WORKDIR/rootfs/lib/firmware/rtlbt/rtl8822_ko.tar.gz" ]] || {
    echo "Error: RTL8822 module archive was not found in the rootfs" >&2
    exit 1
}

# Install reproducible userspace diagnostics after extracting the vendor
# filesystem. The overlay is intentionally additive: vendor startup scripts
# and binaries remain untouched.
if (( INSTALL_DIAGNOSTICS )); then
    cp -a "$ROOTFS_OVERLAY/." "$WORKDIR/rootfs/"
fi

# bluetoothDaemon checks the legacy marker rather than the newer
# /tmp/rtk_hciattach_done path. Without it, launching hcid can be delayed by
# roughly 10 seconds or skipped entirely on some boots.
if (( INSTALL_HCIATTACH_MARKER_COMPAT )); then
    if [[ -f "$WORKDIR/rootfs/script/attach_bluetooth.sh" ]]; then
        sed -i \
            '/^[[:space:]]*rtk_hciattach -s 115200 ttymxc2 rtk_h5$/a\
			touch /tmp/.hciattach_done' \
            "$WORKDIR/rootfs/script/attach_bluetooth.sh"
    fi
fi

# An existing donor signature is retained only when sanitization is disabled.
if (( ! SANITIZE_ROOTFS )); then
    [[ -e "$WORKDIR/rootfs/etc/uuid_sign" ]] ||
        touch "$WORKDIR/rootfs/etc/uuid_sign"
fi

# Let BusyBox getty establish a controlling terminal, then start a root shell
# directly.  This serial login is useful even when ttymxc0 is not registered
# as a kernel console, so install it independently of the module-copy mode.
sed -i -e '/^::askfirst:-\/bin\/sh$/d' \
       -e '/^ttymxc0::respawn:/d' \
    "$WORKDIR/rootfs/etc/inittab"
echo 'ttymxc0::respawn:/sbin/getty -n -l /bin/sh -L ttymxc0 115200 vt100' \
    >> "$WORKDIR/rootfs/etc/inittab"

echo "[5/7] Stripping and packaging reconstructed modules"

if (( ! PRESERVE_VENDOR_MODULES )); then
    mkdir -p "$WORKDIR/modules/general" "$WORKDIR/modules/rtl8822"
    for module in "${MODULE_PATHS[@]}"; do
    name=$(basename -- "$module")
    case "$name" in
        88x2bs.ko|rtk_hci_uart.ko)
            destination="$WORKDIR/modules/rtl8822/$name"
            ;;
        *)
            destination="$WORKDIR/modules/general/$name"
            ;;
    esac
    cp "$KERNEL_BUILD/$module" "$destination"
    "$STRIP" --strip-unneeded "$destination"
        chmod 0644 "$destination"
    done

tar --sort=name --owner=0 --group=0 --numeric-owner -czf \
    "$WORKDIR/rootfs/script/ko.tar.gz" \
    -C "$WORKDIR/modules/general" .
tar --sort=name --owner=0 --group=0 --numeric-owner -czf \
    "$WORKDIR/rootfs/lib/firmware/rtlbt/rtl8822_ko.tar.gz" \
    -C "$WORKDIR/modules/rtl8822" .

# Replace the vendor module tree as well as its boot-time module archives.
# modules.order is the authoritative list for this build; MII and USBNET do
# not occur here because they are linked into vmlinux.
[[ -f "$KERNEL_BUILD/include/generated/utsrelease.h" ]] || {
    echo "Error: generated kernel release header is missing; build the kernel first" >&2
    exit 1
}
KERNEL_RELEASE=$(sed -n 's/^#define UTS_RELEASE "\([^"]*\)"/\1/p' \
    "$KERNEL_BUILD/include/generated/utsrelease.h")
[[ -n "$KERNEL_RELEASE" && "$KERNEL_RELEASE" != */* ]] || {
    echo "Error: invalid kernel release: $KERNEL_RELEASE" >&2
    exit 1
}

if (( ! ROOTFS_MODULES_ONLY )); then
    rm -rf "$WORKDIR/rootfs/lib/modules"
fi
if (( INSTALL_MODULE_TREE )); then
    MODULE_ROOT="$WORKDIR/rootfs/lib/modules/$KERNEL_RELEASE"
    mkdir -p "$MODULE_ROOT/kernel"

    while IFS= read -r module; do
        [[ -n "$module" ]] || continue
        source_module=${module#kernel/}
        [[ -f "$KERNEL_BUILD/$source_module" ]] || {
            echo "Error: modules.order entry is missing: $KERNEL_BUILD/$source_module" >&2
            exit 1
        }
        mkdir -p "$MODULE_ROOT/$(dirname -- "$module")"
        cp "$KERNEL_BUILD/$source_module" "$MODULE_ROOT/$module"
        "$STRIP" --strip-unneeded "$MODULE_ROOT/$module"
        chmod 0644 "$MODULE_ROOT/$module"
    done < "$KERNEL_BUILD/modules.order"

    cp "$KERNEL_BUILD/modules.order" "$MODULE_ROOT/modules.order"
    cp "$KERNEL_BUILD/modules.builtin" "$MODULE_ROOT/modules.builtin"
    chmod 0644 "$MODULE_ROOT/modules.order" "$MODULE_ROOT/modules.builtin"
    depmod -b "$WORKDIR/rootfs" "$KERNEL_RELEASE"
fi
else
    echo "  preserving vendor module archives and /lib/modules tree"
fi

echo "[6/7] Rebuilding and validating the JFFS2 rootfs"
mkfs.jffs2 --little-endian --root="$WORKDIR/rootfs" \
    --eraseblock="$BLOCK_SIZE" --pagesize=4096 \
    --compression-mode="$JFFS2_COMPRESSION_MODE" \
    --pad="$ROOTFS_SIZE" --output="$WORKDIR/rootfs.new.jffs2"

if [[ $(stat -c '%s' "$WORKDIR/rootfs.new.jffs2") -ne $ROOTFS_SIZE ]]; then
    printf 'Error: rebuilt JFFS2 image is 0x%X bytes; partition is 0x%X bytes\n' \
        "$(stat -c '%s' "$WORKDIR/rootfs.new.jffs2")" "$ROOTFS_SIZE" >&2
    exit 1
fi

PYTHONPATH="$JEFFERSON_PYTHONPATH" "$JEFFERSON_BIN" \
    "$WORKDIR/rootfs.new.jffs2" -d "$WORKDIR/rootfs-check" \
    > "$WORKDIR/jefferson-check.log"

for archive in script/ko.tar.gz lib/firmware/rtlbt/rtl8822_ko.tar.gz; do
    expected=$(sha256sum "$WORKDIR/rootfs/$archive" | sed 's/ .*//')
    actual=$(sha256sum "$WORKDIR/rootfs-check/$archive" | sed 's/ .*//')
    [[ "$expected" == "$actual" ]] || {
        echo "Error: rebuilt rootfs verification failed for /$archive" >&2
        exit 1
    }
done
if (( ! PRESERVE_VENDOR_MODULES && INSTALL_MODULE_TREE )); then
    [[ -f "$WORKDIR/rootfs-check/lib/modules/$KERNEL_RELEASE/modules.dep" ]]
    [[ -f "$WORKDIR/rootfs-check/lib/modules/$KERNEL_RELEASE/modules.alias" ]]
    [[ -f "$WORKDIR/rootfs-check/lib/modules/$KERNEL_RELEASE/modules.symbols" ]]
fi
if (( INSTALL_DIAGNOSTICS )); then
    [[ -x "$WORKDIR/rootfs-check/usr/bin/strace" ]]
    expected=$(sha256sum "$ROOTFS_OVERLAY/usr/bin/strace" | sed 's/ .*//')
    actual=$(sha256sum "$WORKDIR/rootfs-check/usr/bin/strace" | sed 's/ .*//')
    [[ "$expected" == "$actual" ]] || {
        echo "Error: rebuilt rootfs verification failed for /usr/bin/strace" >&2
        exit 1
    }
fi
if (( INSTALL_HCIATTACH_MARKER_COMPAT )); then
    if [[ -f "$WORKDIR/rootfs/script/attach_bluetooth.sh" ]]; then
        grep -q '^[[:space:]]*touch /tmp/\.hciattach_done$' \
            "$WORKDIR/rootfs-check/script/attach_bluetooth.sh"
    fi
fi
grep -q '^ttymxc0::respawn:/sbin/getty -n -l /bin/sh -L ttymxc0 115200 vt100$' \
    "$WORKDIR/rootfs-check/etc/inittab"
if (( ! PRESERVE_VENDOR_MODULES && INSTALL_MODULE_TREE )) && \
   find "$WORKDIR/rootfs-check/lib/modules" -mindepth 1 -maxdepth 1 \
        -type d ! -name "$KERNEL_RELEASE" | grep -q .; then
    echo "Error: obsolete kernel-release directory remains in /lib/modules" >&2
    exit 1
fi
if (( SANITIZE_ROOTFS )); then
    [[ ! -e "$WORKDIR/rootfs-check/etc/uuid" ]]
    [[ -f "$WORKDIR/rootfs-check/etc/uuid_sign" ]]
    [[ $(stat -c '%s' "$WORKDIR/rootfs-check/etc/uuid_sign") -eq 256 ]]
    [[ -z $(od -An -v -tu1 "$WORKDIR/rootfs-check/etc/uuid_sign" |
             tr -d ' 0\n') ]]
    [[ -s "$WORKDIR/rootfs-check/etc/device_serial" ]]
    [[ -e "$WORKDIR/rootfs-check/etc/bluetooth/mybluetooth.tar.gz" ]]
    [[ ! -e "$WORKDIR/rootfs-check/root/.ssh/authorized_keys" ]]
    [[ ! -e "$WORKDIR/rootfs-check/var/lib/udhcpd.leases" ]]
else
    [[ -e "$WORKDIR/rootfs-check/etc/uuid_sign" ]]
fi
echo "[7/7] Writing a non-destructive output flash dump"
cp --reflink=auto "$SOURCE_DUMP" "$OUTPUT_DUMP"

# Install the portable environment carrying the original vendor RAM-log
# console, and keep the active catalogue MD5 synchronized with that payload.
dd if="$WORKDIR/environment-update.serial.encrypted" of="$OUTPUT_DUMP" \
   bs=1 seek="$environment_update_offset" count="$environment_update_size" \
   conv=notrunc status=none
read -r catalogue_md5 _ < <(
    dd if="$OUTPUT_DUMP" bs=1 skip="$environment_update_offset" \
       count="$environment_update_size" status=none |
        md5sum
)
[[ $catalogue_md5 =~ ^[[:xdigit:]]{32}$ ]] || {
    echo "Error: failed to calculate environment catalogue MD5" >&2
    exit 1
}
printf '%s' "$catalogue_md5" |
    dd of="$OUTPUT_DUMP" bs=1 seek=$((active_entry + 1)) count=32 \
       conv=notrunc status=none

# Force import_env() to fail on first boot. boot_init() then consumes the
# validated catalogue update, writes the portable environment with its bugs
# marker, and lets the subsequent secure path perform the device DCP setup.
dd if=/dev/zero bs="$BLOCK_SIZE" count=1 status=none | \
    tr '\000' '\377' > "$WORKDIR/environment.invalid"
dd if="$WORKDIR/environment.invalid" of="$OUTPUT_DUMP" \
   bs=1 seek="$ENVIRONMENT_START" count="$BLOCK_SIZE" \
   conv=notrunc status=none

# The installed prefix remains device-key encrypted until the one-time U-Boot
# conversion below. The remainder is stored directly in flash.
dd if="$WORKDIR/zImage.padded" of="$OUTPUT_DUMP" \
   bs=1 skip="$KERNEL_PREFIX_SIZE" seek="$KERNEL_REMAINDER_START" \
   count="$KERNEL_REMAINDER_SIZE" conv=notrunc status=none

for ((index = 0; index < 3; index++)); do
    dd if="$WORKDIR/kernel-$index.encrypted" of="$OUTPUT_DUMP" \
       bs=1 seek=$((STAGING_START + index * BLOCK_SIZE)) \
       count="$BLOCK_SIZE" conv=notrunc status=none
done

dd if="$WORKDIR/rootfs.new.jffs2" of="$OUTPUT_DUMP" \
   bs=1 seek="$ROOTFS_START" count="$ROOTFS_SIZE" \
   conv=notrunc status=none

[[ $(stat -c '%s' "$OUTPUT_DUMP") -eq $FLASH_SIZE ]] || {
    echo "Error: output dump size changed unexpectedly" >&2
    exit 1
}

printf '\nOutput dump:      %s\n' "$OUTPUT_DUMP"
printf 'zImage:           0x%X / 0x%X bytes\n' "$zimage_size" "$KERNEL_SIZE"
printf 'Rootfs:           0x%X bytes at flash 0x%X\n' "$ROOTFS_SIZE" "$ROOTFS_START"
echo
echo "IMPORTANT: do not interrupt power during the first boot/update/reset cycle."
echo "boot_init() will install the catalogue environment and perform DCP conversion."
