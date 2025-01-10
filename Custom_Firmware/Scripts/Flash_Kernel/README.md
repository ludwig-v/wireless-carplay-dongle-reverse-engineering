### Flash 2021.03.06 Kernel (compatible with strace)

Flash 2021.03.06 kernel to fix `PTRACE_TRACEME: Invalid argument` error on hardened one.

⚠️ This kernel is not compatible with newer WiFi chips, **you must have RTL8822BS (Realtek) or LGX8354S (Broadcom)** - newer WiFi chips drivers have been compiled with the hardened kernel and are not retrocompatible -