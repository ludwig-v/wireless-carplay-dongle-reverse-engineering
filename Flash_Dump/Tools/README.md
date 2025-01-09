## Flash Tools

For a flash dump named dump.bin, extract rootFS from the dump by using this command (replace */path/to/*):

    dd if=/path/to/dump.bin of=/path/to/rootfs.jffs2 bs=128 skip=28672

*Details: Skip 28672 (Blocks) * 128 (Block size) = 3670016 bytes, because rootFS starts at 0x380000 inside the flash and write it inside rootfs.jffs2 (total size : 13107200 bytes)*

Follow the development guide to install jefferson via poetry (mandatory, patch needed later): https://github.com/onekey-sec/jefferson/

You can also use my patched fork : https://github.com/ludwig-v/jefferson_carlinkit

**Problem ? Custom JFFS2 implementation has been made by Carlinkit**, they use custom compression algorithm 0x1**6** inside some inodes of the filesystem which is based on Zlib (JFFS2_COMPR_ZLIB = 0x0**6**) but non standard.

**After poking around for hours** I finally understood that they deliberately created this custom method to break files extraction from conventional tools (like *jefferson*) and **they probably did this using a single line of code**.

**Really ? Yes ! They just swapped characters \x32 (space from ASCII table) and \x60  (< from ASCII table)**

Rectifying these characters allows successful Zlib decompression !

If you use original repository of jefferson you must replace *jffs2.py* of jefferson by the patched one present inside this repository, it is adding JFFS2_COMPR_CARLINKIT_ZLIB (0x16) compression method and also provide a fix for symlinks if the directory was not previously created.

You can now extract rootFS by using this command (replace */path/to* to desired path):

    poetry run jefferson /path/to/rootfs.jffs2 -d /path/to/extractedRootFS -f

You can now edit what ever you want. You can restore Custom Firmware behavior for USB drive U2W.sh execution by modifying these files:

 - /etc/mdev/udisk_insert.sh: Replace "utf8=1" by "rw,umask=0000,utf8=1"
 - /etc/mdev/udisk_hotplug.sh: Replace "utf8=1" by "rw,umask=0000,utf8=1"
 - /script/update_box.sh: See below

Add :

    if [ -e /tmp/userspace.log ] ; then
    	cp /tmp/userspace.log /mnt/UPAN/userspace.log && sync # Copy logs to USB key
    fi
    if [ -e /mnt/UPAN/U2W.sh ] ; then
    	sed -i "s/\r//g" /mnt/UPAN/U2W.sh && sync # Remove Windows style CR
    	/mnt/UPAN/U2W.sh > /mnt/UPAN/U2W.txt 2>&1 && sync # Execute custom script and save return
    fi

*Check https://github.com/ludwig-v/wireless-carplay-dongle-reverse-engineering/blob/master/Custom_Firmware/2021.08.24_BASED/CFW.patch*

Repack rootFS this way:

    mkfs.jffs2 --root=/path/to/extractedRootFS --output=/path/to/new_rootfs.jffs2 --eraseblock=64 --little-endian --pad=13107200

Rewrite rootFS inside your original dump by using this command:

    dd if=/path/to/new_rootfs.jffs2 of=/path/to/dump.bin bs=128 seek=28672 conv=notrunc

*Details: Seek inside dump at position 3670016 (rootFS starts at 0x380000), write new_rootfs.jffs2 over it and do not truncate original file*

Reflash the dump on your device using your favorite flash programmer.

If you restored Custom Firmware behavior you can now install Dropbear (SSH), and so on :)

**Enjoy !**