#!/bin/sh
echo 1 > /sys/bus/platform/devices/ci_hdrc.0/inputs/a_suspend_req_inf && /script/iphoneRoleSwitch_test 0x05ac 0x12a8

