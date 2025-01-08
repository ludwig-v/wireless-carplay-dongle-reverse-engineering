########################
# Copyright(c) 2014-2022 DongGuan HeWei Communication Technologies Co. Ltd.
# file    phone_link_deamon.sh
# brief   
# author  Shi Kai
# version 1.0.0
# date    1111月22
########################
#!/bin/bash

Usage() {
	echo "Usage: $0 CarPlay/AndroidAuto/HiCar start/stop"
	exit 1
}

if [ $# -ne 2 ]; then
	Usage
fi

linktype=$1
action=$2

CheckPhoneLinkFile() {
	case $linktype in
		CarPlay)
			test -e /usr/sbin/AppleCarPlay  || exit 1
			;;
		AndroidAuto)
			test -e /usr/sbin/ARMAndroidAuto || exit 1
			;;
		HiCar)
			test -e /usr/sbin/ARMHiCar || exit 1
			;;
		ICCOA)
			test -e /usr/sbin/iccoa || exit 1
			;;
	esac
}

CopyPhoneLinkFile() {
	tmpLibPath=/tmp/lib/
	tmpBinPath=/tmp/bin/
	case $linktype in
		CarPlay)
			test -e $tmpBinPath/AppleCarPlay  || (\
				cp /usr/sbin/AppleCarPlay $tmpBinPath;\
				cp /usr/sbin/ARMiPhoneIAP2 $tmpBinPath;\
				)
			;;
		AndroidAuto)
			test -e $tmpBinPath/ARMAndroidAuto || (\
				cp /usr/sbin/ARMAndroidAuto $tmpBinPath;\
				cp /usr/sbin/hfpd $tmpBinPath;\
				cp -d /usr/lib/libssl.so* $tmpLibPath;\
				)
			;;
		HiCar)
			test -e $tmpBinPath/ARMHiCar || (\
				cp /usr/sbin/ARMHiCar $tmpBinPath;\
				cp /usr/lib/libdmsdp* $tmpLibPath;\
				cp /usr/lib/libHisightSink.so $tmpLibPath;\
				cp /usr/lib/libHwDeviceAuthSDK.so $tmpLibPath;\
				cp /usr/lib/libHwKeystoreSDK.so $tmpLibPath;\
				cp /usr/lib/libmanagement.so $tmpLibPath;\
				cp /usr/lib/libsecurec.so $tmpLibPath;\
				cp /usr/lib/libauthagent.so $tmpLibPath;\
				cp /usr/lib/libnearby.so $tmpLibPath;\
				cp /usr/lib/libhicar.so $tmpLibPath\
				)
			;;
		ICCOA)
			test -e $tmpBinPath/iccoa || (\
				cp /usr/sbin/iccoa $tmpBinPath;\
				cp /usr/sbin/openssl $tmpBinPath;\
				cp -d /usr/lib/libcarlink.so $tmpLibPath;\
				)
			;;
	esac
	sync
}

RunLinkProcess() {
	case $linktype in
		CarPlay)
			ARMiPhoneIAP2
			;;
		AndroidAuto)
			ps |grep -v grep|grep hfpd || hfpd -y -E -f &
			ARMAndroidAuto
			;;
		HiCar)
			ARMHiCar
			;;
		ICCOA)
			iccoa
			;;
	esac
}

EndLinkProcess() {
	case $linktype in
		CarPlay)
			killall ARMiPhoneIAP2
			killall AppleCarPlay
			;;
		AndroidAuto)
			killall hfpd
			killall ARMAndroidAuto
			;;
		HiCar)
			killall ARMHiCar
			;;
		ICCOA)
			killall iccoa
			;;
	esac
}


lockfile=/tmp/.${linktype}_daemon_started
StartLinkDeamon() {
	CheckPhoneLinkFile
	test -e $lockfile && exit 0
	touch $lockfile
	echo "Start Link Deamon: $linktype"
	CopyPhoneLinkFile
	while [ -e $lockfile ]
	do
		if [ -e /sys/class/net/wlan0/address ] && [ ! -e /tmp/bluetoothdaemon_running ]; then
			echo "Wait bt attached success when start phone link daemon!!!"
		else
			RunLinkProcess
		fi
		sleep 1
	done
	echo "End Link Deamon: $linktype"
	exit 0
}

StopLinkDeamon() {
	if [ -e $lockfile ]; then
		echo "Stop Link Deamon: ${linktype}"
		rm -f $lockfile
		EndLinkProcess
	fi
	exit 0
}

case $action in
	start)
		StartLinkDeamon
		;;
	stop)
		StopLinkDeamon
		;;
	*)
		Usage
		;;
esac


