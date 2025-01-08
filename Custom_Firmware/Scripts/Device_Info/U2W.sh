#!/bin/sh

# turn off red light
echo 1 >/sys/class/gpio/gpio2/value;

UUID=`cat /sys/fsl_otp/HW_OCOTP_CFG0 /sys/fsl_otp/HW_OCOTP_CFG1 /sys/fsl_otp/HW_OCOTP_MAC0 /sys/fsl_otp/HW_OCOTP_MAC1 | tr -d '\n' | sed 's/0x//g'`
OLD_UUID=`cat /sys/class/bluetooth/hci0/address /sys/class/net/wlan0/address | md5sum - | awk '{print $1}'`
FACTORY=`cat /etc/factory_name 2>/dev/null | tr -d '\n'`
VERSION=`cat /etc/box_version 2>/dev/null | tr -d '\n'`
VERSION_INT=`printf '%d' "'$VERSION"`
TYPE=`cat /etc/box_product_type | tr -d '\n'`
CUSTOM_TYPE=`cat /tmp/box_custom_type 2>/dev/null | tr -d '\n'`
BRANDS=`cat /etc/support_car_brands 2>/dev/null | tr -d '\n'`
SOFT_VERSION=`cat /etc/software_version 2>/dev/null | tr -d '\n'`
WIFI_CHIP_ID=`cat /sys/bus/sdio/devices/mmc0:0001:1/device`
ACTIVATION_DATA=`cat /etc/uuid_sign 2>/dev/null | tr -d '\n' | base64`

echo "----------------------------------------------------------------"
echo "Device UUID: $UUID"
echo "Device Old UUID: $OLD_UUID"
echo "Device Manufacturer: $FACTORY"
echo "Device Version: $VERSION_INT"
echo "Device Type: $TYPE"
echo "Device Custom Type: $CUSTOM_TYPE"
if [ "$WIFI_CHIP_ID" == "0xb822" ]; then
	echo "Device Wifi: Realtek RTL8822BS"
elif [ "$WIFI_CHIP_ID" == "0xc822" ]; then
	echo "Device Wifi: Realtek RTL8822BS"
elif [ "$WIFI_CHIP_ID" == "0x4354" ]; then
	echo "Device Wifi: Broadcom 4354"
elif [ "$WIFI_CHIP_ID" == "0x4358" ] || [ "$WIFI_CHIP_ID" == "0xaa31" ]; then
	echo "Device Wifi: Broadcom 4358"
elif [ "$WIFI_CHIP_ID" == "0xb733" ]; then
	echo "Device Wifi: Realtek RTL8733BS"
elif [ "$WIFI_CHIP_ID" == "0x9149" ] || [ "$WIFI_CHIP_ID" == "0x9141" ]; then
	echo "Device Wifi: Marvell 8987"
fi
echo "Software Activated Brands: $BRANDS"
echo "Software Version: $SOFT_VERSION"
echo "Activation data (Base64 encoded) - SAVE IT -:"
echo "$ACTIVATION_DATA"

echo "----------------------------------------------------------------"
echo ""

# turn on red light
echo 0 >/sys/class/gpio/gpio2/value;

exit 0
