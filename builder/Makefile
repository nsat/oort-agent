# set variables
# POKY to clone poky to
# REPO to find stuff to copy from
# DEST to build into
# OUTPUT to copy sdk into
# set in targets:
# BRANCH from poky
# ARCH to build
.PHONY: docker armv7a cortexa9 armv8

docker: armv7a cortexa9 armv8

absolutize = $(if $(patsubst /%,,${1}),${CURDIR}/${1},${1})

ABSDEST=$(call absolutize,${DEST})
ABSREPO=$(call absolutize,${REPO})

POKY ?= ${ABSDEST}/poky-${BRANCH}
OUTPUT ?= output

armv7a: export BRANCH=fido
armv7a: export ARCH=armv7a
armv7a: export SDK=poky-glibc-x86_64-meta-toolchain-armv7a-vfp-toolchain-1.8.2.sh
armv7a: sdk
armv7a-docker: armv7a
	docker build . -f ${ABSREPO}/armv7a/Dockerfile --target sdk -t sdk-armv7a

cortexa9: export BRANCH=pyro
cortexa9: export ARCH=cortexa9
cortexa9: export SDK=poky-glibc-x86_64-meta-toolchain-cortexa9thf-neon-toolchain-1.8.2.sh
cortexa9: sdk
cortexa9-docker: cortexa9
	docker build . -f ${ABSREPO}/cortexa9/Dockerfile --target sdk -t sdk-cortexa9

armv8: export BRANCH=sumo
armv8: export ARCH=armv8
armv8: export SDK=poky-glibc-x86_64-meta-toolchain-aarch64-toolchain-2.5.1.sh
armv8: sdk
armv8-docker: armv8
	docker build . -f ${ABSREPO}/armv8/Dockerfile --target sdk -t sdk-armv8

SHELL=/bin/bash

poky: ${POKY}/.git 
${POKY}/.git:
	git clone --depth 1 -b ${BRANCH} https://github.com/nsat/poky ${POKY}

_BUILD_SDK=${ABSDEST}/build/tmp/deploy/sdk/${SDK} 
sdk: ${OUTPUT}/${SDK}

${OUTPUT}/${SDK}: ${_BUILD_SDK}
	cp ${_BUILD_SDK} ${OUTPUT}/

${_BUILD_SDK}: export TEMPLATECONF=${ABSREPO}/${ARCH}/conf
${_BUILD_SDK}: export BUILDDIR=${ABSDEST}/build
${_BUILD_SDK}: poky
	test -d ${ABSDEST}/build/conf || mkdir -p ${ABSDEST}/build/conf
	cp -r ${ABSREPO}/${ARCH}/conf/machine ${ABSDEST}/build/conf
	cp -r ${ABSREPO}/${ARCH}/conf/local.conf ${ABSDEST}/build/conf
	test -d ${OUTPUT} || mkdir -p ${OUTPUT}
	. ${POKY}/oe-init-build-env ${ABSDEST}/build ; \
	test "${SKIP}" == "y" || bitbake meta-toolchain -c populate_sdk
