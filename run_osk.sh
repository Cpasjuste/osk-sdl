#!/bin/sh
export TSLIB_TSDEVICE=/dev/input/event3
export DFBARGS=system=fbdev,no-cursor,disable-module=linux_input
export TERM=xterm
export osk_cmd="./osk-sdl -d a -n a -c osk.conf"

echo "WARNING: This script is meant to assist with debugging osk-sdl NOT for unlocking rootfs!"

make

if [ "$1" = 'g' ]; then
	sudo -E gdb --args ${osk_cmd}
else
	sudo -E ${osk_cmd}
fi
