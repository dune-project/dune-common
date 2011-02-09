#! /bin/bash
# $Id$
# Process this file with autoconf to produce a configure script.

DUNE_AC_INIT # gets module version from dune.module file
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([dune/common/stdstreams.cc])
AM_CONFIG_HEADER([config.h])

# add configure flags needed to create log files for dune-autobuild
DUNE_AUTOBUILD_FLAGS
# check all dune dependecies and prerequisits
DUNE_CHECK_ALL

# preset variable to path such that #include <dune/...> works
AC_SUBST([DUNE_COMMON_ROOT], '$(abs_top_srcdir)')
AC_SUBST([DUNE_COMMON_BIN], '$(abs_top_srcdir)/bin/')
AC_SUBST([AM_CPPFLAGS], '-I$(top_srcdir)')
AC_SUBST([LDADD], '$(top_builddir)/lib/libdunecommon.la')

# write output
AC_CONFIG_FILES([Makefile
     lib/Makefile
     bin/Makefile
     dune/Makefile
     dune/common/Makefile
     dune/common/test/Makefile
     dune/common/exprtmpl/Makefile
     dune/common/parallel/Makefile
     dune/common/parallel/test/Makefile
     doc/Makefile
     doc/doxygen/Makefile
     doc/doxygen/Doxyfile
     doc/buildsystem/Makefile
     m4/Makefile
     am/Makefile
     bin/check-log-store
     dune-common.pc])
AC_OUTPUT

# make scripts executable
chmod +x bin/check-log-store

# print results
DUNE_SUMMARY_ALL
