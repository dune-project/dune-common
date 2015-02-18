am_DATA = inkscape.am webstuff global-rules sourcescheck \
        no-check-without-lib latex checklog doxygen top-rules \
        headercheck documentation cmake-pkg-config

# Not all file names seem to be treated equal by _DATA.
# inkscape.am no-check-without-lib have to be listed in
# EXTRA_DIST to be included into the tarball.
EXTRA_DIST = CMakeLists.txt inkscape.am no-check-without-lib
amdir = $(datadir)/dune-common/am

include $(top_srcdir)/am/global-rules
