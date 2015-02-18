# we need the module file to be able to build via dunecontrol
EXTRA_DIST= CMakeLists.txt dune.module

# don't follow the full GNU-standard
# we need automake 1.9 or newer
AUTOMAKE_OPTIONS = foreign 1.9

SUBDIRS = cmake dune lib share doc bin m4 am

# use configured compiler for "make distcheck"
DISTCHECK_CONFIGURE_FLAGS = CXX="$(CXX)" CC="$(CC)" --enable-parallel=@ENABLE_PARALLEL@ MPICC="$(MPICC)"

include $(top_srcdir)/am/global-rules
include $(top_srcdir)/am/top-rules

# Distribute and install config.h.cmake
configdir = $(datadir)/dune-common
dist_config_DATA = config.h.cmake

include $(top_srcdir)/am/cmake-pkg-config
