#!/bin/sh
# shellcheck disable=SC2039
set -e


# Instructions for generating mouse movement steps for testing keyboard interface clicks:
# 1) start X server, using Xephyr, and a specific screen size:
#	Xephyr -screen 480x800 :11&
# 2) start osk-sdl on the X server:
#	DISPLAY=:11 ./bin/osk-sdl -n a -d a -c osk.conf -v -t
# 3) mouse over desired keys, use xdotool to read cursor coordinates at each position:
#	DISPLAY=:11 xdotool getmouselocation


# Clicks out the string 'qwerty' with the mouse and then clicks 'enter'.
# *** NOTE: Depends on screen size being 480x800 !
mouse_click_qwerty() {
	# q
	xdotool mousemove 21 575 click 1
	# w
	xdotool mousemove 70 575 click 1
	# e
	xdotool mousemove 130 575 click 1
	# r
	xdotool mousemove 185 575 click 1
	# t
	xdotool mousemove 225 575 click 1
	# y
	xdotool mousemove 270 575 click 1
	# enter
	xdotool mousemove 430 775 click 1
}

# Clicks out the string '@#π48' with the mouse and then clicks 'enter'.
# *** NOTE: Depends on screen size being 480x800 !
mouse_click_symbols() {
	# 123 key
	xdotool mousemove 56 771 click 1
	# @ key
	xdotool mousemove 21 651 click 1
	# # key
	xdotool mousemove 92 647 click 1
	# =\< key
	xdotool mousemove 50 690 click 1
	# pi key
	xdotool mousemove 243 591 click 1
	# =\< key
	xdotool mousemove 50 691 click 1
	# 4 key (symbol layout)
	xdotool mousemove 184 576 click 1
	# abc key
	xdotool mousemove 69 745 click 1
	# 8 key (abc layout, 5 row)
	xdotool mousemove 369 557 click 1
	# enter
	xdotool mousemove 463 766 click 1
}

# $1: set to true to run with sudo, else false
# $2: output file for stdout/stderr
# $3: additional opts for osk_sdl
# returns: PID
run_osk_sdl() {
	local use_sudo="$1"
	local out_file="$2"
	local opts="$3 -c osk.conf"
	if [ "$use_sudo" = true ]; then
		# shellcheck disable=SC2086
		sudo ./bin/osk-sdl $opts 2>"$out_file" 1>&2 &
		echo $!
	else
		# shellcheck disable=SC2086
		./bin/osk-sdl $opts 2>"$out_file" 1>&2 &
		echo $!
	fi
}

# $1: result file to read from
# $2: expected contents of result file
# returns: 0 on match, 1 on mismatch
check_result() {
	local result_file="$1"
	local expected="$2"
	local retval
	# check result
	result="$(cat "$result_file")"
	rm "$result_file"
	if [ "$result" != "$expected" ]; then
		printf "ERROR: Unexpected result!\n"
		printf "\t%-15s %s\n" "got:" "$result"
		printf "\t%-15s %s\n" "expected:" "$expected"
		retval=1
	else
		echo "Success!"
		retval=0
	fi
	return $retval
}

#############################################################
# Test key script (-k) with 'mouse' key input (letter layer)
#############################################################
test_keyscript_mouse_letters() {
	echo "** Testing key script with 'mouse' key input (letter layer)"
	local expected="qwerty"
	local result_file="/tmp/osk_sdl_test_mouse_keyscript_$DISPLAY"
	local osk_pid
	osk_pid="$(run_osk_sdl false "$result_file" "-k -n test_disk -d test/luks.disk")"
	sleep 3

	# run test
	mouse_click_qwerty
	sleep 3
	kill -9 "$osk_pid" 2>/dev/null || true

	# check result
	check_result "$result_file" "$expected"
}

##############################################################
# Test key script (-k) with 'mouse' key input (symbol layers)
##############################################################
test_keyscript_mouse_symbols() {
	echo "** Testing key script with 'mouse' key input (symbol layers)"
	local expected="@#π48"
	local result_file="/tmp/osk_sdl_test_mouse_keyscript_$DISPLAY"
	local osk_pid
	osk_pid="$(run_osk_sdl false "$result_file" "-k -n test_disk -d test/luks.disk")"
	sleep 3

	# run test
	mouse_click_symbols
	sleep 3
	kill -9 "$osk_pid" 2>/dev/null || true

	# check result
	check_result "$result_file" "$expected"
}

##################################################
# Test key script (-k) with 'physical' key input
##################################################
test_keyscript_phys() {
	echo "** Testing key script with 'physical' key input"
	local result_file="/tmp/osk_sdl_test_phys_keyscript_$DISPLAY"
	local expected="postmarketOS"
	local osk_pid
	osk_pid="$(run_osk_sdl false "$result_file" "-k -n test_disk -d test/luks.disk")"
	sleep 3

	# run test
	xdotool type --delay 300 "$expected"
	xdotool key Return
	sleep 3
	kill -9 "$osk_pid" 2>/dev/null || true

	# check result
	check_result "$result_file" "$expected"
}

test_keyscript_mouse_letters
test_keyscript_mouse_symbols
test_keyscript_phys
