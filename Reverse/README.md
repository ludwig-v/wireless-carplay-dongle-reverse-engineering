**Work in progress:** reversing new binary (appeared in 2021.03.06) to understand the new encryption mechanism.

*.img > .tar.gz generated using the binary via QEMU*

**> Reading blocks of 81920 bytes and decrypting AES using Hardware-Accelerated AES driver**


> ls -l /dev/hwaes

    crw-rw----    1 root     root       10,   0 Jan  1  1970 /dev/hwaes

> readlink /sys/dev/char/10\:0

    ../../devices/virtual/misc/hwaes

> ls -l /sys/devices/virtual/misc/hwaes

    -r--r--r--    1 root     root          4096 Jan  2 00:03 dev
    drwxr-xr-x    2 root     root             0 Jan  2 00:03 power
    lrwxrwxrwx    1 root     root             0 Jan  2 00:03 subsystem -> ../../../../class/misc
    -rw-r--r--    1 root     root          4096 Jan  2 00:03 uevent

> cat /sys/devices/virtual/misc/uevent

    MAJOR=10
    MINOR=0
    DEVNAME=hwaes

> ls -l /sys/devices/virtual/misc/hwaes/power

    -rw-r--r--    1 root     root          4096 Jan  2 00:03 autosuspend_delay_ms
    -rw-r--r--    1 root     root          4096 Jan  2 00:03 control
    -r--r--r--    1 root     root          4096 Jan  2 00:03 runtime_active_time
    -r--r--r--    1 root     root          4096 Jan  2 00:03 runtime_status
    -r--r--r--    1 root     root          4096 Jan  2 00:03 runtime_suspended_time

> stat /dev/hwaes

      File: /dev/hwaes
      Size: 0         	Blocks: 0          IO Block: 4096   character special file
    Device: 5h/5d	Inode: 323         Links: 1     Device type: a,0
    Access: (0660/crw-rw----)  Uid: (    0/    root)   Gid: (    0/    root)
    Access: 1970-01-01 00:00:00.000000000
    Modify: 1970-01-01 00:00:00.000000000
    Change: 1970-01-01 00:00:02.000000000

