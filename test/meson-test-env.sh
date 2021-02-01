#!/bin/sh

# There are multiple implementations of xvfb-run out there:
# - https://git.archlinux.org/svntogit/packages.git/tree/trunk/xvfb-run?h=packages/xorg-server
# - https://salsa.debian.org/xorg-team/xserver/xorg-server/-/blob/debian-unstable/debian/local/xvfb-run
#
# They don't always support the same options. --auto-display seems to only work reliably on Arch, whereas the debian
# version (used by Alpine Linux too) doesn't have that option at all, and uses --auto-servernum.
#
# This script will grep -h for --auto-display and prefer that if it's available, else it'll fall back to
# --auto-servernum
set -x
auto_param="--auto-servernum"
if xvfb-run -h | grep -q auto-display; then
	auto_param="--auto-display"
fi

xvfb-run "$auto_param" --server-args="-ac -screen 0 480x800x24" -- "$@"
