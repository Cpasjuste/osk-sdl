#!/bin/sh
#
# Copyright (C) 2017 Clayton Craft <clayton@craftyguy.net>
# This file is part of osk-sdl.
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

export TSLIB_TSDEVICE=/dev/input/event3
export DFBARGS=system=fbdev,no-cursor,disable-module=linux_input
export TERM=xterm
export osk_cmd="./bin/osk-sdl -d a -n a -c osk.conf"

echo "WARNING: This script is meant to assist with debugging osk-sdl and NOT"
echo "for unlocking rootfs!"

make

if [ "$1" = 'g' ]; then
  sudo -E gdb --args ${osk_cmd}
else
  sudo -E ${osk_cmd}
fi
