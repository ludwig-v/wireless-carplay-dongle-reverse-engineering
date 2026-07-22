#####################
# Copyright(c) 2014-2022 DongGuan HeWei Communication Technologies Co. Ltd.
# file    sync_box_time.sh
# brief
# author  hcw
# version 1.0.0
#####################
#!/bin/bash

function leapYear
{
	if [ $(($YEAR%4)) -eq 0 ]&&[ $(($YEAR%100)) -eq 0 ]||[ $(($YEAR%400)) -eq 0 ];then
		ISLEAP=1
	else
		ISLEAP=0
	fi
}

SEC=59
while [ $SEC -eq 59 ]
do
	DATE=`curl -I www.paplink.cn/server.php |grep 'Date'`
#	DATE="Date: Mon, 30 Apr 2023 17:35:01 GTM"

	TIME=`echo $DATE|awk '{print $6}'`
	DAY=`echo $DATE|awk '{print $3}'`
	MON=`echo $DATE|awk '{print $4}'`
	YEAR=`echo $DATE|awk '{print $5}'`
	HOUR=`echo $TIME|awk -F ':' '{print $1}'`
	MIN=`echo $TIME|awk -F ':' '{print $2}'`
	SEC=`echo $TIME|awk -F ':' '{print $3}'`

	case $MON in
		Jan)
	    		MON="01"
		;;
		Feb)
	    		MON="02"
		;;
		Mar)
	    		MON="03"
		;;
		Apr)
	    		MON="04"
		;;
		May)
	    		MON="05"
		;;
		Jun)
	    		MON="06"
		;;
		Jul)
	    		MON="07"
		;;
		Aug)
	    		MON="08"
		;;
		Sep)
	    		MON="09"
		;;
		Oct)
	    		MON="10"
		;;
		Nov)
	    		MON="11"
		;;
		Dec)
	    		MON="12"
		;;
	esac

	BIGMON="01 03 05 07 08 10 12"
	LITMON="04 06 09 11"
	TWOMON="02"

	HOUR=`expr $HOUR + 8`
	if [ $HOUR -gt 24 ];then
		HOUR=`expr $HOUR - 24`
		DAY=`expr $DAY + 1`
		if [[ "${BIGMON[@]}" =~ "$MON" ]]&&[[ $DAY -gt 31 ]];then
			DAY=1
			MON=`expr $MON + 1`
			if [ $MON -gt 12 ];then
				MON=1
				YEAR=`expr $YEAR + 1`
			fi
		elif [[ "${LITMON[@]}" =~ "$MON" ]]&&[[ $DAY -gt 30 ]];then
			DAY=1
			MON=`expr $MON + 1`
		elif [[ "${TWOMON}" =~ "$MON" ]];then
			leapYear
			echo $ISLEAP
			if [ $ISLEAP -eq 1 ]&&[ $DAY -gt 29 ];then
				DAY=1
				MON=`expr $MON + 1`
			elif [ $ISLEAP -eq 0 ]&&[ $DAY -gt 28 ];then
				DAY=1
				MON=`expr $MON + 1`
			fi
		fi	
	fi

	SEC=`expr $SEC + 1`
	if [ $SEC -eq 60 ];then
		SEC=59
		sleep 2
	fi
done

if [ $SEC -lt 10 ];then
	SEC="0"${SEC}
fi
if [ $HOUR -lt 10 ];then
	HOUR="0"${HOUR}
fi
if [ $DAY -lt 10 ];then
	DAY="0"${DAY}
fi

echo  "set box time: ${YEAR}-${MON}-${DAY} ${HOUR}:${MIN}:${SEC}"

sleep 0.5
date -s "${YEAR}-${MON}-${DAY} ${HOUR}:${MIN}:${SEC}"
