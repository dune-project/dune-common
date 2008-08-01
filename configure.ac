#! /bin/bash
# $Id$
# Process this file with autoconf to produce a configure script.

DUNE_AC_INIT # gets module version from dune.module file
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([common/stdstreams.cc])
AM_CONFIG_HEADER([config.h])

# create symlink dune -> $top_srcdir
DUNE_SYMLINK
# add configure flags needed to create log files for dune-autobuild
DUNE_AUTOBUILD_FLAGS
# check all dune dependecies an prerequisits
DUNE_CHECK_ALL

# preset variable to path such that #include <dune/...> works
AC_SUBST([DUNE_COMMON_ROOT], '$(top_builddir)')
AC_SUBST([AM_CPPFLAGS], '-I$(top_srcdir)')
AC_SUBST([LOCAL_LIBS], '$(top_builddir)/common/libcommon.la')

# write output
AC_CONFIG_FILES([Makefile
     lib/Makefile
     bin/Makefile
     common/Makefile
     common/test/Makefile
     common/exprtmpl/Makefile
     doc/Makefile
     doc/devel/Makefile
     doc/layout/Makefile
     doc/doxygen/Makefile
     doc/doxygen/Doxyfile
     doc/buildsystem/Makefile
     m4/Makefile
     am/Makefile
     bin/wmlwrap
     bin/check-log-store
     dune-common.pc])
AC_OUTPUT

# make scripts executable
chmod +x bin/wmlwrap
chmod +x bin/check-log-store

# print results
DUNE_SUMMARY_ALL

echo
echo Note: Most of the libraries checked for above are only used for the self-test
echo of Dune.  The library itself will build and the headers will work even if
echo ALBERTA, MPI, etc. cannot be found.  An exception to this are UG and AmiraMesh
echo which need to be found right now if you want to use them later.
echo
