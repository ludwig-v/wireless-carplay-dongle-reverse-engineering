## SPI Flash Structure

| Start | Length | Detail |
| - | - | - |
| 0x000000 | 0x000400 | Initial load padding for SPI |
| 0x000400 | 0x000400 | IVT |
| 0x000800 | 0x000400 | IVT |
| 0x000C00 | 0x000400 | Boot data |
| 0x001000 | 0x02F000 | DCD |
| 0x030000 | 0x010000 | uBoot |
| 0x040000 | 0x340000 | Kernel (zImage) |
| 0x380000 | 0xC80000 | rootFS (JFFS2) |

AutoBox.img inside /tmp/ folder of all updates is a perfect copy of Flash memory from 0x0 to 0x380000 (uBoot + Kernel)