#! /bin/bash
# $Id$
# Process this file with autoconf to produce a configure script.

# get module version from dune.module file
m4_define([DUNE_MOD_VERSION],
  [m4_esyscmd([grep ^Version: dune.module | tr '\n' ' ' | sed -e 's/^.*://' -e 's/\s*//'])])
AC_INIT(dune-common, DUNE_MOD_VERSION, dune@dune-project.org)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([common/stdstreams.cc])
AM_CONFIG_HEADER([config.h])

# check all dune-module stuff
DUNE_CHECK_ALL_M

# preset variable to path such that #include <dune/...> works
AC_SUBST([DUNE_COMMON_ROOT], '$(top_srcdir)')
AC_SUBST([AM_CPPFLAGS], '-I$(top_srcdir)')
AC_SUBST([LOCAL_LIBS], '$(top_builddir)/common/libcommon.la')

DUNE_SUMMARY_ALL

echo
echo Note: Most of the libraries checked for above are only used for the self-test
echo of Dune.  The library itself will build and the headers will work even if
echo ALBERTA, MPI, etc. cannot be found.  An exception to this are UG and AmiraMesh
echo which need to be found right now if you want to use them later.
echo

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
     doc/buildsystem/Makefile
     m4/Makefile
     am/Makefile
     bin/wmlwrap
     bin/check-log-store
     dune-common.pc])
AC_OUTPUT

chmod +x bin/wmlwrap
chmod +x bin/check-log-store
