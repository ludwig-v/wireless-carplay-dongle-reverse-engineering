########################
#File:    getFuncModule.sh
#author:  hcw
#version: 1.0.0
#date:    17May23
########################
#bin/bash

STARTMODULE='Boot128'
USBMODULE='USBDrv'
COMMODULE='Common'
SERMODULE='Service'
UPGMODULE='Upg'
WIFICHIPPATH='/sys/bus/sdio/devices/mmc0:0001:1/device'
CUSVERPATH='/etc/box_version'
WEBMODULE='WebPUBLIC'

for line in `cat $WIFICHIPPATH`
do
	CHIPTYPE=$line
done

case $CHIPTYPE in
	0xb822)
		WIFIMODULE='RTL8822BS' #wifi驱动模块
	;;
	0x4354)
		WIFIMODULE='BCM4354'
	;;
	0x9149)
		WIFIMODULE='SD8987'
	;;
	0x4358)
		WIFIMODULE='BCM4358'
	;;
	0x4335)
		WIFIMODULE='BCM4335'
	;;
	0xc822)
		WIFIMODULE='RTL8822CS'
	;;
	0x9159)
		WIFIMODULE='IW416'
	;;
esac

INPARA=`cat /etc/box_product_type`
#product_type没有下划线的客制化版本加上PUBLIC
if [[ $(echo ${INPARA} | grep "_") ]]; then
	CUSMODULE="Cus_${INPARA}"
else	
	CUSMODULE="Cus_${INPARA}_PUBLIC" #客户UI模块
fi
ISA15=${INPARA: 0: 1} #是否为A15系列
if [ "$ISA15" = "A" ]
then
	if [ -z "${INPARA##*Auto_Box*}" ]
	then
		MDLINK='CarPaly,AndroidAuto,AndroidMirror,iOSMirror' #手机互联模块
	elif [ -z "${INPARA##*A15W*}" ] #A15W
	then
		MDLINK='CarPaly,AndroidAuto,AndroidMirror,iOSMirror,HiCar'
	elif [ -z "${INPARA##*A15H*}" ] #A15H
	then
		MDLINK='CarPlay,HiCar'
	fi
else
	Cusver=`hexdump ${CUSVERPATH} | awk '{print $2}'|sed -n '1p'` #获取客户版本
#	echo ${Cusver}
	case $Cusver in 
		00ff)
			WEBMODULE='WebPUBLIC'
		;;
		0002)
			if [ $(echo ${INPARA} | grep "JUPAZIP") ]; then
				WEBMODULE='WebJUPAZIP'
			else
				WEBMODULE='WebAUTOKIT'
			fi
		;;
		0005)
			WEBMODULE='WebDDDCAT'
		;;
	esac
	HUType=${INPARA%%2*} #判定第一个'2'前面的字符以获取车机互联类型
#	echo ${HUType}
	case $HUType in
		U)
			HULINK='HUCP' #车机互联模块
			if [ -z "${INPARA##*U2AC*}" ] #U2AC支持两种车机互联
			then
				HULINK='HUCP,HUAA'
				MDLINK='CarPlay,AndroidAuto'
			elif [ -z "${INPARA##*U2AW*}" ] #U2AW
			then
				MDLINK='CarPlay,AndroidAuto'
			elif [ -z "${INPARA##*U2W*}" ] #U2W
			then
				MDLINK='CarPlay'
			elif [ -z "${INPARA##*U2HC*}" ] #U2HC
			then
				MDLINK='CarPlay,HiCar'
			elif [ -z "${INPARA##*U2HW*}" ] #U2HW
			then
				MDLINK='HiCar'
			elif [ -z "${INPARA##*U2IW*}" ] #U2IW
			then
				MDLINK='CarPlay,ICCOA'
			fi
		;;
		UC)
			WEBMODULE='WebC2X'
			#${INPARA} == *"_"*
			#${ISPUB} == *"D"*
			#${INPARA} == *"D"*
			if [[ $(echo ${INPARA} | grep "_") ]]; then  #'_'是product_type的子串
				ISPUB=${INPARA%%"_"*}
				if [[ $(echo ${ISPUB} | grep "D") ]]; then  #判定'D'是否是子串
					HULINK='HUBDCL'
				else
					HULINK='HUCL'
				fi
			else
				if [[ $(echo ${INPARA} | grep "D") ]]; then  #判定'D'是否是子串
					HULINK='HUBDCL'
				else
					HULINK='HUCL'
				fi
			fi
			if [ -z "${INPARA##*UC2W*}" ] #UC2W
			then
				MDLINK='CarPlay'
			elif [ -z "${INPARA##*UC2HW*}" ] #UC2HW
			then
				MDLINK='HiCar'
			elif [ -z "${INPARA##*UC2HC*}" ] #UC2HC
			then
				MDLINK='CarPlay,HiCar'
			elif [ -z "${INPARA##*UC2AW*}" ] #UC2AW
			then
				MDLINK='AndroidAuto'
			elif [ -z "${INPARA##*UC2CA*}" ] #UC2CA
			then
				MDLINK='AndroidAuto,CarPlay'
			fi
		;;
		O)
			HULINK='HUAA' #车机互联模块
			if [ -z "${INPARA##*O2W*}" ] #O2W
			then
				MDLINK='AndroidAuto'
			fi
	esac
		
fi
	echo "${STARTMODULE},${USBMODULE},${COMMODULE},${SERMODULE},${UPGMODULE},${HULINK},${MDLINK},$WIFIMODULE,${WEBMODULE},${CUSMODULE}"

