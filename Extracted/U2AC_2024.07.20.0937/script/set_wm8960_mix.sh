########################
# Copyright(c) 2014-2016 DongGuan HeWei Communication Technologies Co. Ltd.
# file    start_mic_record.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    19Aug16
########################
#!/bin/bash
#open mic mixer and set to max volume
tinymix 0 60 60
tinymix 2 1 0
tinymix 35 180 180
#left
#tinymix 5 4
#tinymix 6 4
#tinymix 8 1
#tinymix 49 1
#tinymix 53 1 
#tinymix 54 1 
tinymix 55 0 
#right
#tinymix 3 7 #R3
tinymix 4 7 #R2
tinymix 7 3 #R1
tinymix 48 1 #Right Boost
tinymix 50 1 #R2
tinymix 51 0 #R3
tinymix 52 1 #R1
