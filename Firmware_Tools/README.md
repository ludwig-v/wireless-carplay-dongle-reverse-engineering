### Firmware Tools

### ⚠️ Please note that this packing / unpacking method is not working anymore since firmware 2021.03.06 (U2W - Carlinkit 1.0 / 2.0 / 3.0) and new binary have yet to be reverse engineered.

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
Unpack images from rc.dll and rcvec.dll files (from ui.tar.gz):

    php UI_DLL.php
Repack edited images to new/rc.dll and new/rcvec.dll files (adding new images is not supported yet):

    php UI_DLL.php pack
