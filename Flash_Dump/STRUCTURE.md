## SPI Flash Structure

| Start | Length | Detail |
| - | - | - |
| 0x000000 | 0x000400 | Padding 0x00 (Initial Load Region Size for SPI) |
| 0x000400 | 0x000C00 | Config (imximage.cfg) |
| 0x001000 | 0x00002B | IVT |
| 0x001020 | 0x00000C | Boot Data |
| 0x00102C | 0x0001E0 | DCD |
| 0x00120C | 0x000624 | Padding 0x00 |
| 0x001830 | 0x02A7D0 | uBoot |
| 0x02C000 | 0x014000 | CSF, Certs, Signatures |
| 0x040000 | 0x340000 | Kernel (zImage) |
| 0x380000 | 0xC80000 | rootFS (JFFS2) |

AutoBox.img inside /tmp/ folder of all updates is a almost copy of Flash memory from 0x0 to 0x380000 (uBoot + Kernel)
When flashing another dump the zone from 0x030000 to 0x070000 is updated by the device

---

![Structure](https://boundarydevices.com/wp-content/uploads/2020/11/uboot_signed-1.jpg)

### IVT Format

*The IVT has the following format where each entry is a 32-bit word:*
| Data name | Value | Detail |
| - | - | - |
| **header** | D1 00 20 40 | Header Tag (0xD1) / Header Length (0x0020) / Header version (0x40) |
| **entry** | 00 00 80 87 | Absolute address of the first instruction to execute from the image (uBoot) |
| **reserved1** | 00 00 00 00 | Reserved and should be zero |
| **dcd** | FC F7 7F 87 | Absolute address of the image DCD. The DCD is optional so this field may be set to NULL if no DCD is required. See Device Configuration Data (DCD) for further details on the DCD. |
| **boot data** | F0 F7 7F 87 | Absolute address of the boot data |
| **self** | D0 F7 7F 87 | Absolute address of the IVT. Used internally by the ROM |
| **csf** | D0 A7 82 87 | Absolute address of the Command Sequence File (CSF) used by the HAB library. See High-Assurance Boot (HAB) for details on the secure boot using HAB. This field must be set to NULL if a CSF is not provided in the image |
| **reserved2** | 00 00 00 00 | Reserved and should be zero |

### Boot Data Format

*The boot data must follow the format defined in the table found here, each entry is a 32- bit word.*
| Data name | Value | Detail |
| - | - | - |
| **start** | D0 E7 7F 87 | Absolute address of the image |
| **length** | 00 D0 02 00 | Size of the program image |
| **plugin** | 00 00 00 00 | Plugin flag (see Plugin image) |

### DCD Format

*The DCD header is 4 B with the following format:*
| Data name | Value |
| - | - |
| **Tag** | 0xD2 |
| **Length** | 0x01E0 |
| **Version** | 0x40 |