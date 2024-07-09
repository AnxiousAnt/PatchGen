SECTION = "x11"
PRIORITY = optional
DESCRIPTION = "Pure Data anywhere"
HOMEPAGE = "http://gige.xdv.org/pda/"
LICENSE = "GPL"
PACKAGES = "${PN}"

PR = "r1"

DEPENDS = "tk"
SRC_URI = "http://gige.xdv.org/pda/release/src/PDa-${PV}-src.tgz;md5sum=b0375f214cc46c04d207307eb2a9708b"

S = "${WORKDIR}/PDa/src"
FILES_${PN} = "${bindir}/pd \
			${bindir}/pdsend \
			${bindir}/pdreceive \
			${libdir}/pd/bin/* \
			${libdir}/pd/intern/*.pd_linux \
			${libdir}/pd/extra/*.pd_linux \
			${libdir}/pd/doc/* "

do_compile () {
    export BROOT=${STAGING_DIR}
    export X11LIB=${STAGING_LIBDIR}
    oe_runmake -C ${S} all
}
do_install() {
  oe_runmake -C ${S} DESTDIR=${D} install
}
