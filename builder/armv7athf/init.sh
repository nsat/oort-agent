#!/bin/sh
. /opt/poky/1.8.2/environment-setup-armv7athf-vfp-poky-linux-gnueabi
export CMAKE_SYSROOT=$SDKTARGETSYSROOT
exec "$@"
