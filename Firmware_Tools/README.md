### Firmware Tools

### Firmware encrypt / decrypt for firmware >= 2021.03.06 (AES encrypted tarball archive)

    bash Firmware<FIRMWARE_TYPE>.sh <encrypt|decrypt> <input_file> <output_file>

Replace **<FIRMWARE_TYPE>** by the firmware type you are working with (also works for HWFS "modules" files)

| U2W | U2AW | U2AC |
|--|--|--|
| V1 V2 V3 | V4 | V5 |


### Firmware encrypt / decrypt for firmware < 2021.03.06 (Obfuscated tarball archive, U2W only)

Convert a stock firmware .img to .tar.gz:

    php U2W_Decrypt.php U2W_Update.img
Extract firmware content to "U2W_Update" folder:

    mkdir -p ./U2W_Update && tar xvf U2W_Update.tar.gz -C ./U2W_Update
Fix permissions before packing a .tar.gz firmware archive:

    chown -R 1000:1000 ./U2W_Update
Pack "U2W_Update" folder to .tar.gz:

    tar -czf ./U2W_Update.tar.gz -C ./U2W_Update .
Convert a tar.gz firmware archive to .img:

    php U2W_Crypt.php U2W_Update.tar.gz

### UI modding

Unpack images from rc.dll and rcvec.dll files (from ui.tar.gz):

    php UI_DLL.php
Repack edited images to new/rc.dll and new/rcvec.dll files (adding new images is not supported yet):

    php UI_DLL.php pack
