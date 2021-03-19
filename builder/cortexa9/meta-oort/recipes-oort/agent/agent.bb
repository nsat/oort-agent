PROVIDES = "oort"
LICENSE = "CLOSED"

DEPENDS += "cmake-native"
RDEPENDS_${PN} += "glibc"

S = "${WORKDIR}/oort"

SRC = "/work/oort-ta-dev/agent"

do_configure() {
	[ -d "${S}" ] || mkdir ${S}
	# ln -s ${SRC}/ ${S}/agent
	rsync -a ${SRC}/ ${S}/agent/
        cd ${S}/agent
        make clean-objs clean-cmake cmake
}

do_compile() {
    cd ${S}/agent
    make full LIBDIR=${STAGING_LIBDIR}
}

do_install() {
    install -d ${TMPDIR}/oort
    install ${S}/agent/build/oort-server ${TMPDIR}/oort/agent
}

