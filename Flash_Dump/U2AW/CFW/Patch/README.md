Inside rootFS folder:

    ln -s libz.so.1.2.11 ./usr/lib/libz.so

    ln -s ../sbin/sftp-server ./usr/libexec/sftp-server

    chmod 775 -R ./usr/sbin

    chmod 775 -R ./usr/lib

    patch -s -p0 < patchCFW.diff 
