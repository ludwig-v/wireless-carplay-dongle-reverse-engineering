# March 2021 U-Boot Style Firmware Boot & Update Process

*2022 U-Boot is slightly different and has not been reversed yet.*

## Memory Map

| Location | Purpose |
|---|---|
| Scratch RAM buffer (~2 KB) | Holds the decrypted environment block plus a null terminator |
| RAM staging address (referred to as the "read buffer") | Receives raw data read from flash; also used in place for decryption |
| RAM output address (referred to as the "decompression buffer") | Receives the LZO1X-decompressed 64 KB image, made up of the restored `device.dtb` (device tree blob) plus the default U-Boot environment |
| Marker location at the end of the 64 KB read buffer | Holds a 4-byte marker string used as a boot-mode flag |
| Marker location at the end of the decompression buffer | Where the same marker is written into a freshly built image before it's committed to flash |
| Flash offset holding the active 64 KB boot block | Contains the encrypted environment and, in flashing/normal-boot mode, a bootable firmware image |
| Flash offset holding the update catalogue | A small table listing candidate update chunks |

## Step-by-Step Flow

### 1. Allocate a working buffer

A small scratch buffer is allocated in RAM to later hold the decrypted
environment data.

### 2. Probe the SPI flash

The bootloader probes the SPI flash device, using a clock speed of
40,000,000 Hz (40 MHz). If the probe fails, the bootloader falls back to a
set of default environment values and aborts the routine — there's no
flash to boot from or update.

### 3. Read the boot block

A fixed 64 KB region is read from a defined offset in flash into the RAM
read buffer. This block is expected to contain an encrypted environment
and, depending on mode, an encrypted firmware image.

### 4. Check the boot-mode marker

The last four bytes of the 64 KB block are compared against a fixed ASCII
marker string. This isn't a literal bug report — it's a magic value used
purely as a state flag, written by the bootloader itself once a valid image
has been committed (see step 7.5).

- If the marker is present, the bootloader takes the **factory/upgrade flashing path**.
- If the marker is absent, it takes the **normal boot path**.

### 5a. Factory/upgrade flashing path

When the marker is present:

- The bootloader provisions the flashing-related fuses on the SoC if they
  haven't been burned yet. This is exactly what you'd expect from a
  factory-flashing or field-upgrade routine: the fuse state needs to be
  set up the first time a unit goes through this path (e.g. at the
  factory, or on a unit's first upgrade), and once fuses are burned the
  board delays briefly and resets so the change takes effect, rather than
  continuing further in this boot cycle.
- The 64 KB block is decrypted in place using AES in CBC mode, with what
  is referred to here as "cipher mode 8."
- The first portion of the decrypted block (the environment data) is
  copied into the scratch buffer and imported as the active environment.
- A small housekeeping patch is applied to one environment variable
  (`heweiencrypt`): any occurrence of the substring `mm d 0x` within its
  value is rewritten to `mm u 0x`. In other words, a `(d)ecrypt` command
  token is replaced with a `(u)pdate` token — most likely rewriting the
  encrypted data in place with an update-mode command rather than
  decrypting and displaying it, consistent with this being the
  flashing/update path rather than a normal read path.
- After the patch, the bootloader delays and resets the board so it comes
  back up with the corrected environment in place.

### 5b. Normal boot path

When the marker is absent:

- The bootloader checks a secondary, narrower pattern within the same
  marker bytes. If that secondary pattern matches, an internal flag is set
  that will later cause the bootloader to skip the update-check logic and
  boot the existing image directly.
- The 64 KB block is decrypted in place using AES in CBC mode, but with a
  different cipher configuration, referred to here as "cipher mode 2" —
  using per-unit CFG0/CFG1 fuse values as the IV, with the actual key
  hardware-locked inside the processor.
- The environment portion of the decrypted block is copied into the
  scratch buffer, to be imported in the next shared step.

### 6. Import the environment

Whichever path was taken, the bootloader now imports the environment from
the scratch buffer, and checks the result: a **non-zero** result means the
import **succeeded**, and **zero** means it **failed**. This matches
observed device behavior — deliberately invalidating or erasing the
environment is what actually forces the device down the
firmware-update/restore path described below, so "no usable environment"
has to be the branch that leads there, not the "success" branch.

- **If the import succeeds** (non-zero result), the bootloader re-confirms
  flashing fuse provisioning, and then continues on to normal U-Boot
  execution using the environment that was just imported (e.g. running
  `bootcmd` and whatever else the environment specifies) — this part
  happens beyond the excerpt covered here, not a hard stop. The environment
  is already good, so there's nothing to restore.
- **If the import fails** (zero result — e.g. the environment is missing,
  erased, or corrupted), the bootloader checks the internal flag set in
  step 5b:
  - If the flag is set, it attempts a direct jump into the firmware via
    `flash->boot()`, and — whether or not that jump succeeds — falls
    through to a reset. In practice the device comes back up and boots
    normally after this reset, rather than the `flash->boot()` call itself
    being the thing that boots the device. No LED pattern is shown here.
  - If the flag is **not** set, the bootloader proceeds to the **firmware
    update path** — this is the actual recovery/restore mechanism that
    kicks in when the environment can't be imported at all.

### 7. Firmware update path

This is the logic that checks for and applies a pending firmware update.

**7.1 — Read the update catalogue.** A small (1 KB) table is read from a
fixed flash offset into RAM. This table lists candidate update entries.

**7.2 — Locate the active entry.** Each catalogue entry occupies a fixed
41-byte slot. The bootloader scans these slots looking for one whose first
byte is a `?` character, which marks it as the active/pending update.

**7.3 — Extract the update chunk's location.** From the matched entry, the
bootloader reads a flash offset and a byte count describing where the
actual (still encrypted, still compressed) update payload lives, and reads
that chunk into the RAM read buffer.

**7.4 — Decrypt and decompress.** The chunk is decrypted in place using the
same "cipher mode 8" used for the flashing-path environment. The first four
bytes of the decrypted payload give the length of an embedded LZO1X-compressed
stream, which immediately follows. That stream is decompressed into a fixed
64 KB output buffer. This is how the device's `device.dtb` (device tree
blob) and the default U-Boot environment get restored — the compressed
stream packages both together, and decompressing it reconstructs the
factory-default device tree and environment that this chunk represents.

**7.5 — Commit the restored block, if complete.** If decompression produced
the full expected 64 KB (a basic integrity check), the bootloader:

- Re-encrypts the restored `device.dtb` + default-environment block using a
  third cipher configuration, referred to here as "cipher mode 9" — most
  likely the encrypt-direction counterpart of mode 2, using the same
  hardware-locked key and CFG0/CFG1-derived IV, so that the block can later
  be read back correctly via mode 2 on a normal boot.
- Writes the same boot-mode marker string into the tail of this restored
  block. This is what causes the *next* boot cycle to take the
  factory/upgrade flashing path (step 5a) rather than looking for another
  update.
- Writes the newly encrypted block back to the same flash offset used for
  the 64 KB boot block, with verification.

**7.6 — Finish.** An LED status pattern plays for a fixed duration (~6
seconds), signaling that the update is in progress, and the board resets —
coming back up into either the freshly written restored block, or simply
retrying the boot sequence if no valid update entry was found in step 7.2.
This 6-second animation is decoupled from the actual work: writing the
restored 64 KB block back to flash (across its constituent sectors/blocks)
completes in well under a second. The animation duration looks like it's
there purely to give the user a visible, consistent "update in progress"
indication, not to reflect how long the write itself takes.

### 8. Cleanup

Regardless of which path was taken (barring an early abort), the scratch
buffer is freed and the flash handle is cleared before the routine exits.

## Summary of the Decision Logic

1. Probe flash. If this fails, load defaults and stop.
2. Read the 64 KB boot block.
3. Check the boot-mode marker.
   - Present → factory/upgrade flashing path: provision flashing fuses if
     needed, decrypt with mode 8, import environment, patch the
     `heweiencrypt` variable (decrypt → update token), reset.
   - Absent → normal boot path: check a secondary pattern (sets a
     "direct boot" flag), decrypt with mode 2.
4. Import the environment (note: non-zero = success, zero = failure here).
   - Succeeds → re-confirm fuse provisioning, continue on to normal U-Boot
     execution using the imported environment (beyond this excerpt).
   - Fails, direct-boot flag set → boot existing firmware directly, reset.
   - Fails, flag not set → run the firmware update/restore path.
5. Firmware update path: read catalogue, find the pending entry, decrypt
   and decompress the referenced chunk, and if it decompresses cleanly,
   re-encrypt it, mark it, and write it back to flash. Reset either way.

## Summary Diagram

```
                boot_init() start
                       │
               probe SPI flash (40 MHz)
                       │
             read 64 KB boot block from flash
                       │
             check boot-mode marker (tail bytes)
             ┌─────────┴──────────┐
        marker found         marker absent
      (mode 8 / static         (mode 2 / device-
       "flashing" key)          bound via CFG0/CFG1)
             │                        │
  factory/upgrade flashing path:      normal boot path:
   provision fuses if needed,   check secondary pattern
   decrypt, import env, patch   (sets direct-boot flag),
   heweiencrypt var, reset      decrypt, copy env
             │                            │
             └──────────┬─────────────────┘
                        │
                 import environment
              (non-zero = success,
               zero = failure)
             ┌──────────┴──────────┐
        import OK             import fails
             │                      │
      re-confirm fuse       ┌───────┴────────┐
      provisioning,         │                 │
      continue to           │                 │
      normal U-Boot   direct-boot flag     flag not set
      execution using  set → attempt        → firmware update
      imported env     flash->boot,         path (read catalogue,
      (beyond this      reset (device then   find '?' entry,
      excerpt)          boots normally,      decrypt (mode 8) +
                        no LED here)         LZO1X decompress,
                                             if complete: re-encrypt
                                             (mode 9), write marker,
                                             flash write-with-verify,
                                         LED (update pattern),
                                         reset)
```

## Notable Design Points

- **The two "read" cipher modes reflect two different key models**:
  - **Mode 8** (used when the marker is present, i.e. the "bugs"/flashing
    state) most likely uses a **static key** — the same key across devices.
    This fits its role in the firmware-flashing/update workflow, where an
    update image is prepared once and needs to be decryptable by any unit
    running this bootloader.
  - **Mode 2** (used on the normal boot path) most likely uses
    per-unit **CFG0/CFG1** fuse values as the AES **initialization vector
    (IV)**, not as the key itself. The actual key is believed to reside
    inside the processor (e.g. in on-chip OTP/eFuse-protected storage) and
    is not accessible or readable from software. This still ties the
    cipher output to the specific hardware unit — since the IV varies
    per-device — while keeping the underlying key hardware-locked.
  - **Mode 9** (used only when re-encrypting a freshly restored
    `device.dtb` + environment block before writing it back to flash) is
    most likely the **encrypt-direction counterpart of mode 2** — the same
    underlying key (hardware-locked, inside the processor) and the same
    CFG0/CFG1-derived IV scheme, just run in the encrypt direction instead
    of decrypt. That would explain why the restored block ends up
    readable again via mode 2 on a subsequent normal boot: it was written
    with the matching encrypt operation of that same cipher.
- **The marker string doubles as a state flag**, not just a validity check:
  its presence indicates the block is in flashing/update-sourced (static
  key) form, while its absence indicates a device-bound (fused) image that
  should be treated as already trusted for this specific unit.
- **The update catalogue is a simple fixed-width table**: 41-byte slots
  scanned for a `?` sentinel byte, each pointing to an offset/size pair
  describing where the actual encrypted, compressed update payload sits in
  flash.