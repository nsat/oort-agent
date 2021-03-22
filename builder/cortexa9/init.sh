#!/bin/sh
. /opt/poky/2.3.2/environment-setup-cortexa9hf-neon-poky-linux-gnueabi
export CMAKE_SYSROOT=$SDKTARGETSYSROOT
exec "$@"
