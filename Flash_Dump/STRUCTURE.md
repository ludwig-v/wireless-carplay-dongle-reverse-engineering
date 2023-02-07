## SPI Flash Structure

| Start | Length | Detail |
| - | - | - |
| 0x0000000 | 0x400 | Initial load padding for SPI |
| 0x0000400 | 0x400 | IVT |
| 0x0000800 | 0x400 | IVT |
| 0x0000C00 | 0x400 | Boot data |
| 0x0001000 | 0x2F000 | DCD |
| 0x0030000 | 0x10000 | uBoot |
| 0x0040000 | 0x340000 | Kernel (zImage) |
| 0x0380000 | 0xC80000 | rootFS (JFFS2) |