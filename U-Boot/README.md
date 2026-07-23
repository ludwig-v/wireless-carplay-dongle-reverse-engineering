# U-Boot Style Firmware Boot & Update Process

This document is an attempt to fully describe the Carlinkit U-Boot boot
process, based on the **March 2021** U-Boot build. The 2022 U-Boot has not
been reverse engineered as of this writing, but it appears the key material
was changed and that an invalid/erased environment no longer triggers the
firmware-update path on its own — it most likely now requires the marker
described below as well.

## Memory Map

| Address / Offset | Size | Purpose |
|---|---|---|
| RAM `0x80800000` | up to 64 KB | "Read buffer" — receives raw data read from flash; also decrypted in place here |
| RAM `~0x80800000` (scratch) | `0x820` bytes | Scratch buffer — holds the copied-out environment block (`0x800` bytes) plus a null terminator |
| RAM `0x80810000` | 64 KB | "Decompression buffer" — receives the LZO1X-decompressed image: restored `device.dtb` + default U-Boot environment |
| RAM `0x8080FF00` (tail of read buffer) | 4 bytes at `0xFC`–`0xFF` | Boot-mode marker read from the freshly-read 64 KB block |
| RAM `0x8081FF00` (tail of decompression buffer) | 4 bytes at `0xFC`–`0xFF` | Boot-mode marker written into the newly restored block before flashing it back |
| Flash `0x30000` | 64 KB (`0x10000`) | The active boot block — encrypted environment, and in flashing/normal-boot mode, a bootable firmware image |
| Flash `0x800` | 1 KB (`0x400`) | Update catalogue — table of candidate update entries |
| Flash (variable, from catalogue entry) | variable (`val2`) | Encrypted/compressed update chunk for the active entry |

## Key Markers, Offsets & Constants

| Item | Value | Meaning |
|---|---|---|
| SPI probe clock | `0x2625A00` = 40,000,000 Hz | 40 MHz SPI clock speed used to probe the flash device |
| Primary marker string | `"bugs"` (bytes `'b','u','g','s'` at offsets `0xFC..0xFF`, reversed order `0xFF='b'`, `0xFE='u'`, `0xFD='g'`, `0xFC='s'`) | Flashing-mode marker — presence routes to the factory/upgrade flashing path (mode 8, static key) |
| Secondary pattern | `'b'` at `0xFF`, `'b'` at `0xFD`, `'b'` at `0xFC` (note: `0xFE` is *not* checked) | "bbb" pattern — sets the direct-boot flag (`r4`) on the normal-boot path |
| Environment block size | `0x800` bytes | Size of the environment portion copied out of the decrypted 64 KB block |
| Env variable of interest | `heweiencrypt` | A U-Boot env variable (like `bootcmd`) holding a command sequence — for 4 kernel blocks + 1 env block, decrypts a static-key (mode 8) backup copy from the end of the kernel partition, then re-encrypts it device-tied (mode 9) and writes it to its normal front-of-partition location; the rest of the kernel is unencrypted |
| Cipher mode 8 key | ASCII `https://apple.co` (16 bytes) | Static, hardcoded AES-128 key for mode 8 |
| Cipher mode 8 IV | ASCII `m/kb/HT208050   ` (16 bytes, incl. 3 trailing spaces) | Static, hardcoded AES-128 IV for mode 8 |
| Direct-boot call | `flash->boot(0, 0x10000, 0)` | Attempted jump target — offset `0x10000` into the boot block |
| Catalogue entry size | `0x29` (41) bytes | Fixed record size in the update catalogue |
| Catalogue scan bound | `0x8080019A` | End address for the catalogue-entry scan loop (`0x80800000 + 0x400 - ...`) |
| Catalogue sentinel | `'?'` | First byte of an entry marks it as the active/pending update slot |
| Entry field: flash offset | at entry `+0x21`, 4 bytes (`val1`) | Flash offset of the update chunk (example dump value: `0x0002D000`) |
| Entry field: chunk size | at entry `+0x25`, 4 bytes (`val2`) | Byte length of the update chunk (example dump value: `0x00002800`) |
| Example `'?'` entry offset | `0x8F6` (relative, in the sample dump) | Where the matched catalogue entry was found in that particular capture |
| LZO1X compressed length | first 4 bytes of decrypted chunk (example: `0x252B` / 9515) | Little-endian length of the LZO1X stream that follows |
| Decompression output size | fixed `0x10000` (64 KB) | Required output size for the decompression to be considered complete |
| Update-path LED duration | `0x1770` = 6000 (ms) | Fixed ~6 second animation; actual flash write of the restored block completes in well under a second |

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

A fixed 64 KB region is read from flash offset `0x30000` into the RAM read
buffer at `0x80800000`. This block is expected to contain an encrypted
environment and, depending on mode, an encrypted firmware image.

### 4. Check the boot-mode marker

The last four bytes of the 64 KB block (RAM `0x8080FF00`, offsets
`0xFC`–`0xFF`) are compared against the fixed ASCII marker string `"bugs"`.
This isn't a literal bug report — it's a magic value used purely as a state
flag, written by the bootloader itself once a valid image has been
committed (see step 7.5).

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
  value is rewritten to `mm u 0x`. In other words, every `(d)ecrypt` command
  token in it is replaced with an `(u)pdate` token.
- After the patch, the bootloader calls `boot()` on the *patched*
  `heweiencrypt` value. `heweiencrypt` is a U-Boot environment variable
  holding a command sequence, the same way `bootcmd` does. 
  `boot()` executes that command sequence directly out of
  the environment, using the `u` variant of its commands rather than the
  original `d` variant.
- After that sequence runs, the bootloader delays and resets the board so
  it comes back up with the corrected environment (and, per below, a
  partially re-flashed kernel) in place.

#### What the `heweiencrypt` env variable actually does

Based on a captured example of the (pre-patch) `heweiencrypt` value, it
repeats the same four-command sequence for **4 blocks belonging to the
kernel**, plus **1 additional block for the U-Boot environment + `device.dtb`**
(5 blocks total; three of the kernel blocks are shown below):

```
sf probe 0
sf read 0x80800000 0x3439c0 0x10000 ; mm d 0x80800000 0x10000 ; mm m 0x80800000 0x10000 ; sf update 0x80800000 0x40000 0x10000
sf read 0x80800000 0x3539c0 0x10000 ; mm d 0x80800000 0x10000 ; mm m 0x80800000 0x10000 ; sf update 0x80800000 0x70000 0x10000
sf read 0x80800000 0x3639c0 0x10000 ; mm d 0x80800000 0x10000 ; mm m 0x80800000 0x10000 ; sf update 0x80800000 <...> 0x10000
```

Per block, the pattern is:

1. `sf read <ram> <src_offset> 0x10000` — read a 64 KB block from flash
   into RAM at `0x80800000`. These source blocks live at the **end of the
   kernel partition** (`0x3439c0`, `0x3539c0`, `0x3639c0`, ...) — a
   static-key-encrypted backup copy of the kernel's leading blocks (and,
   for the fifth block, of the default U-Boot environment + `device.dtb`)
   , rather than their normal in-place location.
2. `mm d 0x80800000 0x10000` (pre-patch) — decrypt that block in place
   using **cipher mode 8** (the static key/IV pair documented below).
3. `mm m 0x80800000 0x10000` — **re-encrypts** the now-plaintext block
   using **cipher mode 9**, the device-tied key (the same underlying
   hardware key and CFG0/CFG1-derived IV scheme as mode 2, just in the
   encrypt direction). This is what actually resolves the earlier
   uncertainty about what `mm m` does.
4. `sf update <ram> <dst_offset> 0x10000` — write the resulting
   **device-tied re-encrypted** block back to flash at its real,
   operational destination — the front of the kernel partition, where the
   bootloader/kernel normally expects to find it. Destination offsets
   aren't evenly spaced like the source offsets: the first two are
   `0x40000` then `0x70000` (a `0x30000` jump, not `0x10000`), so the
   destination layout likely reflects gaps for other partitions/regions.
   `sf update` only rewrites sectors that actually differ from the buffer,
   making repeated runs of this sequence cheap once the destination
   already holds the correctly re-encrypted data.

So the four kernel blocks (the kernel's original GZIP header and initial
data) and the one U-Boot environment block are never left as plaintext at
rest — they go from **static-key encrypted** (mode 8, factory/shared) to
**device-tied encrypted** (mode 9, this unit's own key), converting a
mass-producible, identically-encrypted factory image into a per-device
one. Everything else in the kernel image, beyond these four blocks, is
stored unencrypted and untouched by this whole process.

Read this way, the `d` → `u` patch is a one-time **factory/first-boot
provisioning step**: the unpatched `d` ("decrypt-only") form would just
decrypt each block into RAM without touching flash, whereas the patched
`u` ("update") form additionally re-encrypts with the device's own key and
persists that result to the correct in-partition location via `sf update`.
Run once during this factory/upgrade flashing path, it converts the
statically-encrypted backup blocks into permanent, device-bound ones —
which is also consistent with why the device switches to the `"bugs"`
marker / normal boot path afterward: from then on, the kernel-header
blocks and the environment are both readable with mode 2 (the matching
decrypt-direction cipher for this unit), not the factory static key.

#### Cipher mode 8 key material

The AES key and IV used for cipher mode 8 have been recovered as ASCII
strings, each 16 bytes (matching AES-128):

- **Key**: the ASCII string `https://apple.co` (16 characters), hex-encoded
- **IV**: the ASCII string `m/kb/HT208050   ` (16 characters, including 3
  trailing spaces to pad to 16 bytes), hex-encoded

Both are static, hardcoded strings rather than anything derived from
per-device fuse state — consistent with mode 8 being the "flashing/static
key" mode used for factory/upgrade provisioning (as opposed to mode 2,
which is CFG0/CFG1-fuse-derived and device-bound). The key and IV appear
to be built from a real Apple URL and support-article identifier
(`https://apple.co/...HT208050`) split across the two fields — an apparent
attempt to hide the static key by disguising it as an innocuous-looking
URL/reference fragment rather than an obviously random-looking string.

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

**7.1 — Read the update catalogue.** A 1 KB table is read from flash offset
`0x800` into RAM at `0x80800000`. This table lists candidate update entries.

**7.2 — Locate the active entry.** Each catalogue entry occupies a fixed
`0x29` (41-byte) slot. The bootloader scans these slots, from `0x80800000`
up to `0x8080019A`, looking for one whose first byte is a `?` character,
which marks it as the active/pending update. (In the sample dump, the `?`
was found at relative offset `0x8F6`.)

**7.3 — Extract the update chunk's location.** From the matched entry, the
bootloader reads a 4-byte flash offset at entry `+0x21` and a 4-byte byte
count at entry `+0x25` — describing where the actual (still encrypted,
still compressed) update payload lives — and reads that chunk into the
RAM read buffer at `0x80800000`. (Sample dump values: offset `0x0002D000`,
size `0x00002800`.)

**7.4 — Decrypt and decompress.** The chunk is decrypted in place using the
same "cipher mode 8" used for the flashing-path environment. The first four
bytes of the decrypted payload give the length of an embedded LZO1X-compressed
stream (sample dump: `0x252B` / 9515 bytes), which immediately follows.
That stream is decompressed into a fixed 64 KB output buffer at
`0x80810000`. This is how the device's `device.dtb` (device tree blob) and
the default U-Boot environment get restored — the compressed stream
packages both together, and decompressing it reconstructs the
factory-default device tree and environment that this chunk represents.

**7.5 — Commit the restored block, if complete.** If decompression produced
the full expected `0x10000` (64 KB), the bootloader:

- Re-encrypts the restored `device.dtb` + default-environment block using a
  third cipher configuration, referred to here as "cipher mode 9" — most
  likely the encrypt-direction counterpart of mode 2, using the same
  hardware-locked key and CFG0/CFG1-derived IV, so that the block can later
  be read back correctly via mode 2 on a normal boot.
- Writes the `"bugs"` marker string into the tail of this restored block,
  at `0x8081FF00` (offsets `0xFC`–`0xFF`). This is what causes the *next*
  boot cycle to take the factory/upgrade flashing path (step 5a) rather
  than looking for another update.
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
     `heweiencrypt` script (decrypt-only → decrypt+persist), run it to
     decrypt (mode 8, static key) and re-encrypt (mode 9, device-tied) the
     4 kernel blocks + 1 env block, reset.
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
  - **Mode 9** is the **encrypt-direction counterpart of mode 2** — the
    same underlying key (hardware-locked, inside the processor) and the
    same CFG0/CFG1-derived IV scheme, just run in the encrypt direction
    instead of decrypt. It shows up in two places: re-encrypting a freshly
    restored `device.dtb` + environment block in the firmware-update path
    (step 7.5), and re-encrypting the 4 kernel blocks + 1 env block that
    `heweiencrypt` converts from static-key to device-tied form. Both
    cases end up readable again via mode 2 on a subsequent normal boot,
    since it's the matching encrypt operation of that same cipher.
- **The marker string doubles as a state flag**, not just a validity check:
  its presence indicates the block is in flashing/update-sourced (static
  key) form, while its absence indicates a device-bound (fused) image that
  should be treated as already trusted for this specific unit.
- **The update catalogue is a simple fixed-width table**: 41-byte slots
  scanned for a `?` sentinel byte, each pointing to an offset/size pair
  describing where the actual encrypted, compressed update payload sits in
  flash.
