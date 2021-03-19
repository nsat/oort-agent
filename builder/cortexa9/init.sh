#!/bin/sh
. /opt/poky/1.8.2/environment-setup-cortexa9thf-neon-poky-linux-gnueabi
export CMAKE_SYSROOT=$SDKTARGETSYSROOT
exec "$@"
