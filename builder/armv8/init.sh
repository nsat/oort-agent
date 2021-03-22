#!/bin/sh
. /opt/poky/2.5.1/environment-setup-aarch64-poky-linux
export CMAKE_SYSROOT=$SDKTARGETSYSROOT
exec "$@"
