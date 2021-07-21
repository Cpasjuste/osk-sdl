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

# Clicks the 'osk' toggle button.
# *** NOTE: Depends on screen size being 480x800 !
mouse_click_osk_toggle() {
	xdotool mousemove 460 775 click 1
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
	local opts="$3 -c $OSK_SDL_CONF_PATH"
	if [ "$use_sudo" = true ]; then
		# shellcheck disable=SC2086
		sudo "$OSK_SDL_EXE_PATH" $opts 2>"$out_file" 1>&2 &
		echo $!
	else
		# shellcheck disable=SC2086
		"$OSK_SDL_EXE_PATH" $opts 2>"$out_file" 1>&2 &
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

########################################################
# Test key script (-k) with 'physical' key input, and -x
########################################################
test_keyscript_no_keyboard_phys() {
	echo "** Testing key script with 'physical' key input"
	local result_file="/tmp/osk_sdl_test_phys_keyscript_$DISPLAY"
	local expected="postmarketOS"
	local osk_pid
	osk_pid="$(run_osk_sdl false "$result_file" "-k -n test_disk -d test/luks.disk -x")"
	sleep 3

	# run test
	xdotool type --delay 300 "$expected"
	xdotool key Return
	sleep 3
	kill -9 "$osk_pid" 2>/dev/null || true

	# check result
	check_result "$result_file" "$expected"
}

########################################################
# Test key script (-k) with 'physical' key input, and -x
########################################################
test_keyscript_mouse_toggle_osk() {
	echo "** Testing osk toggle button and 'mouse' key input"
	local expected="qwerty"
	local result_file="/tmp/osk_sdl_test_mouse_toggle_keyscript_$DISPLAY"
	local osk_pid
	osk_pid="$(run_osk_sdl false "$result_file" "-k -n test_disk -d test/luks.disk -x")"
	sleep 3

	# run test
	mouse_click_osk_toggle
	sleep 1
	mouse_click_qwerty
	sleep 3
	kill -9 "$osk_pid" 2>/dev/null || true

	# check result
	check_result "$result_file" "$expected"
}

##################################################
# Test luks unlocking
##################################################
test_luks_phys() {
	# This test requires a privileged user (root, for devicemapper)
	if [ "$(whoami)" != "root" ]; then
		echo "This test requires elevated privileges, skipping."
		exit 77
	# CI_JOB_ID is set by gitlab CI
	elif [ "$CI_JOB_ID" ]; then
		echo "This test does not work in the gitlab CI, skipping."
		exit 77
	fi

	echo "** Testing luks unlocking with 'physical' key input"
	local test_disk="test/luks.disk"
	local passphrase="postmarketOS"
	local result_file="/tmp/osk_sdl_test_phys_luks_$DISPLAY"
	local osk_pid
	local retval=0

	# create test luks disk
	qemu-img create -f raw "$test_disk" 20M 1>/dev/null
	echo "$passphrase" | sudo cryptsetup --iter-time=1 luksFormat "$test_disk"

	# run osk-sdl
	osk_pid="$(run_osk_sdl true "$result_file" "-n osk-sdl-test -d $test_disk")"
	sleep 3

	# run test
	xdotool type --delay 300 "$passphrase"
	xdotool key Return
	sleep 3
	kill -9 "$osk_pid" 2>/dev/null || true

	# check result
	if [ ! -b /dev/mapper/osk-sdl-test ]; then
		echo "ERROR: Decrypted test device not created!"
		retval=1
	else
		echo "Success!"
	fi

	# clean up
	sudo cryptsetup close osk-sdl-test || true
	rm $test_disk || true

	return $retval
}

if [ -z "$OSK_SDL_EXE_PATH" ]; then
	echo "\$OSK_SDL_EXE_PATH must be set to the path of the osk-sdl binary to test"
	exit 1
fi

if [ -z "$OSK_SDL_CONF_PATH" ]; then
	echo "\$OSK_SDL_CONF_PATH must be set to the path of the osk.conf to use in testing"
	exit 1
fi

# make sure osk-sdl uses X and not some other video backend for testing
export SDL_VIDEODRIVER=x11

case "$1" in
	test_keyscript_phys)
		test_keyscript_phys
		;;
	test_keyscript_no_keyboard_phys)
		test_keyscript_no_keyboard_phys
		;;
	test_keyscript_mouse_letters)
		test_keyscript_phys
		;;
	test_keyscript_mouse_symbols)
		test_keyscript_phys
		;;
	test_luks_phys)
		test_luks_phys
		;;
	test_keyscript_mouse_toggle_osk)
		test_keyscript_mouse_toggle_osk
		;;
	*)
		test_keyscript_phys
		test_keyscript_no_keyboard_phys
		test_keyscript_mouse_letters
		test_keyscript_mouse_symbols
		test_keyscript_mouse_toggle_osk
		test_luks_phys
		;;
esac
