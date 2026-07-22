#!/usr/bin/env python3
"""Extract the DTB stored at offset 0x800 in a Carlinkit environment update."""

import argparse
import struct
import subprocess
from pathlib import Path

import lzo


CATALOGUE_OFFSET = 0x800
CATALOGUE_SCAN_SIZE = 0x19A
CATALOGUE_ENTRY_SIZE = 0x29
KEY = b"https://apple.co"
IV = b"m/kb/HT208050   "


def openssl_decrypt(data):
    command = [
        "openssl", "enc", "-d", "-aes-128-cbc", "-K", KEY.hex(),
        "-iv", IV.hex(), "-nopad",
    ]
    return subprocess.run(command, input=data, stdout=subprocess.PIPE,
                          check=True).stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path, help="16 MiB SPI flash dump")
    parser.add_argument("output", type=Path, help="output DTB")
    parser.add_argument("--dts", type=Path, help="also decompile to this DTS")
    parser.add_argument("--dtc", default="dtc")
    args = parser.parse_args()

    image = args.input.read_bytes()
    if len(image) != 0x1000000:
        raise SystemExit("input is not a 16 MiB SPI flash dump")

    entry = None
    for relative in range(0, CATALOGUE_SCAN_SIZE, CATALOGUE_ENTRY_SIZE):
        candidate = CATALOGUE_OFFSET + relative
        if image[candidate] == ord("?"):
            entry = candidate
            break
    if entry is None:
        raise SystemExit("active catalogue environment entry was not found")

    update_offset, update_size = struct.unpack_from("<II", image, entry + 0x21)
    encrypted = image[update_offset:update_offset + update_size]
    payload = openssl_decrypt(encrypted)
    compressed_size = struct.unpack_from("<I", payload)[0]
    environment = lzo.decompress(payload[4:4 + compressed_size], False, 0x10000)

    offset = 0x800
    if environment[offset:offset + 4] != b"\xd0\x0d\xfe\xed":
        raise SystemExit("DTB magic was not found at environment offset 0x800")
    size = struct.unpack_from(">I", environment, offset + 4)[0]
    if size <= 0x28 or offset + size > len(environment):
        raise SystemExit("environment DTB has an invalid totalsize")

    args.output.write_bytes(environment[offset:offset + size])
    if args.dts:
        with args.dts.open("wb") as output:
            subprocess.run([args.dtc, "-I", "dtb", "-O", "dts",
                            args.output], stdout=output, check=True)
    print(f"Extracted DTB {size:#x} from catalogue update {update_offset:#x}+"
          f"{update_size:#x}")


if __name__ == "__main__":
    main()
