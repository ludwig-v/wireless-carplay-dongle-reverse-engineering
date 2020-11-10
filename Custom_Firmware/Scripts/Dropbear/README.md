# Overview

This script runs a [Dropbear SSH](https://github.com/mkj/dropbear) server which allows root access to the dongle.

**Run the script only if you really know what you are doing. You can permanently brick your dongle. We accept no responsibility for loss or damage!**

# Usage

- Uncompress the public and private keys
```
tar -xvzf keys.tar.gz
```

- Copy the public and privates key to your `<home>/.ssh`:
```
cp cplay2air ~/.ssh
cp cplay2air.pub ~/.ssh
chmod 700 ~/.ssh/cplay2air
chmod 700 ~/.ssh/cplay2air.pub
```

- Copy the following files to the root of a USB drive:
```
U2W.sh
dropbear
cplay2air.pub
```

- Power on the dongle, and insert the USB drive. The red power light will turn off to let you know Dropbear is running.

- Connect to the WiFi network created by dongle, (`GMxx`, `AutoBox-xxxx`, etc)
  - If prompted for a password, enter `12345678`
  - (If using a Mac, the password would populated via iCloud)

- `SSH` into the dongle with the following command:
```
ssh -i ~/.ssh/cplay2air root@192.168.50.2
```

- If you receive a host warning, type `yes`:
```
The authenticity of host '192.168.50.2 (192.168.50.2)' can't be established. 
RSA key fingerprint is SHA256:9qg+0BArUwa183yEEwf22sZB+YKh6kzHJj90smg1EYg. 
Are you sure you want to continue connecting (yes/no/[fingerprint])? yes
Warning: Permanently added '192.168.50.2' (RSA) to the list of known hosts.
```

- You should now be logged in to the dongle as `root`:
```
[240] Jan 02 00:22:08 lastlog_perform_login: Couldn't stat /var/log/lastlog: No such file or directory
[240] Jan 02 00:22:08 lastlog_openseek: /var/log/lastlog is not a file or directory!
[(\t)\u@\w]#
```

- To verify enter the `uname -a` command: 
```
[(\t)\u@\w]#uname -a
Linux sk_mainboard 3.14.52+g94d07bb #39 SMP PREEMPT Tue Oct 27 01:47:06 CST 2020 armv7l GNU/Linux
```
