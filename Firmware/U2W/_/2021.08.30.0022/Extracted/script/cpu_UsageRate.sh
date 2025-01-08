##########
#!/bin/bash
#LINE=0
#MAXLINE=10000
OUTFILENAME=/tmp/cpu_UsageRate.csv
#TIMEFILE=/tmp/.cputime.txt
TOPFILE=/tmp/.cpu_topmsg.txt
INTERVAL=8
#csvtime &
riddle_top -m 3 -d $INTERVAL &
sleep $INTERVAL
sleep $INTERVAL
echo "CPUTOTAL(%),TOP1(%),TOP2(%),TOP3(%),TEMP(℃),FREQ(kHz),TIME(s),MEM(%)" > $OUTFILENAME
while true
do
cat $TOPFILE |awk 'NR==1 {printf $2+$4","};\
NR==4{if($5 ~/[[:punct:]]./){printf substr($8$9$10,1,10)":"$2","} else {printf substr($9$10$11,1,10)":"$2","}};\
NR==5{if($5 ~/[[:punct:]]./){printf substr($8$9$10,1,10)":"$2","} else {printf substr($9$10$11,1,10)":"$2","}};\
NR==6{if($5 ~/[[:punct:]]./){printf substr($8$9$10,1,10)":"$2","} else {printf substr($9$10$11,1,10)":"$2","}}'\
>>$OUTFILENAME
cat /sys/class/thermal/thermal_zone0/temp|awk '{printf substr($0/1000,1,4)"," ;}'>>$OUTFILENAME
sleep 2
cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq|awk '{printf $0/1000"," ;}'>>$OUTFILENAME
date |awk '{printf substr($4,1,2) ;printf substr($4,4,2);printf substr($4,7,2)","}'>>$OUTFILENAME
##cat $TIMEFILE |awk '{printf $0"," ;}' >>$OUTFILENAME
cat /proc/meminfo | awk 'NR==1 {p1=$2};NR==2 {p2=$2};END {print substr((p1-p2)/p1*100,1,4)}' >>$OUTFILENAME
sleep $INTERVAL
#LINE=`wc -l $OUTFILENAME|awk '{printf $1}'`
#if [ $LINE -gt $MAXLINE ]
#	then
#		sed -i '2,100d' $OUTFILENAME
#fi
done
