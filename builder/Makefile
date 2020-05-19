# set variables
# POKY to clone poky to
# REPO to find stuff to copy from
# DEST to build into
# OUTPUT to copy sdk into
# set in targets:
# BRANCH from poky
# ARCH to build
.PHONY: docker armv7a armv8

docker: armv7a armv8

absolutize = $(if $(patsubst /%,,${1}),${CURDIR}/${1},${1})

ABSDEST=$(call absolutize,${DEST})
ABSREPO=$(call absolutize,${REPO})

POKY ?= ${ABSDEST}/poky-${BRANCH}
OUTPUT ?= output

armv7a: export BRANCH=fido
armv7a: export ARCH=armv7a
armv7a: sdk
armv7a-docker: armv7a
	docker build ${ABSREPO}/armv7a --target sdk-armv7a -t sdk-armv7a

armv8: export BRANCH=sumo
armv8: export ARCH=armv8
armv8: sdk
armv8-docker: armv8
	docker build ${ABSREPO}/armv8 --target sdk-armv8 -t sdk-armv8

SHELL=/bin/bash

poky: ${POKY}/.git 
${POKY}/.git:
	git clone --depth 1 -b ${BRANCH} https://github.com/nsat/poky ${POKY}

sdk: poky
	test -d ${POKY}/meta-oort || cp -r ${ABSREPO}/${ARCH}/meta-oort ${POKY}/
	test -d ${ABSDEST}/build || mkdir -p ${ABSDEST}/build
	test -d ${OUTPUT} || mkdir -p ${OUTPUT}
	TEMPLATECONF=${ABSREPO}/${ARCH}/conf ; \
	BUILDDIR=${ABSDEST}/build ; \
	. ${POKY}/oe-init-build-env ${ABSDEST}/build ; \
	test "${SKIP}" == "y" || bitbake meta-toolchain -c populate_sdk
	cp ${ABSDEST}/build/tmp/deploy/sdk/*.sh ${OUTPUT}/
