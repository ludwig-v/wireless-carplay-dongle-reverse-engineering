#!/bin/sh

audioCodec=none
i2cdetect -y -a 0 0x1a 0x1a | grep "1a" && audioCodec=wm8960
if [ $audioCodec == "wm8960" ]; then
        echo "audio codec: wm8960"
        test -e /tmp/snd-soc-wm8960.ko && insmod /tmp/snd-soc-wm8960.ko
        test -e /tmp/snd-soc-imx-wm8960.ko && insmod /tmp/snd-soc-imx-wm8960.ko
		/script/set_wm8960_mix.sh
else
        echo "audio codec: btsco"
        test -e /tmp/snd-soc-bt-sco.ko && insmod /tmp/snd-soc-bt-sco.ko
        test -e /tmp/snd-soc-imx-btsco.ko && insmod /tmp/snd-soc-imx-btsco.ko
fi

