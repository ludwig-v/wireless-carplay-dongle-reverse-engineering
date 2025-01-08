########################
# Copyright(c) 2014-2020 DongGuan HeWei Communication Technologies Co. Ltd.
# file    once.sh
# brief   
# author  Haiguang Yin
# version 1.0.0
# date    14Apr20
########################
#!/bin/bash
echo "Start upg"
rm /tmp/*.img;rm /tmp/update.tar.gz;mv /tmp/update/tmp/* /tmp/;chmod +x /tmp/upg;/tmp/upg
echo "End upg"
