# $Id$
# Process this file with autoconf to produce a configure script.
AC_INIT(dune, 0.0, dune@hal.iwr.uni-heidelberg.de)
AM_INIT_AUTOMAKE(dune, 0.0, dune@hal.iwr.uni-heidelberg.de)
AC_CONFIG_SRCDIR([grid/common/grid.hh])

# do almost nothing, an application using Dune should provide the
# checks (use the macro provided in m4/dune.m4 to do this)

# Checks for programs.
AC_PROG_CXX

# don't build shared libs per default, this is way better for debugging...
AC_DISABLE_SHARED
AC_PROG_LIBTOOL

# write output
AC_CONFIG_FILES([Makefile
	         common/Makefile
                 fem/Makefile
                 fem/common/Makefile
                 fem/discfuncarray/Makefile
                 fem/fastquad/Makefile
                 fem/gaussquadrature/Makefile
                 fem/operator/Makefile
                 grid/Makefile
                 grid/albertgrid/Makefile
                 grid/common/Makefile
                 grid/sgrid/Makefile
                 grid/yaspgrid/Makefile
                 lib/Makefile
                 lib/io/Makefile
                 operator/Makefile
                 solver/Makefile
                 solver/common/Makefile
                 solver/pmgsolver/Makefile
		 doc/Makefile
		 m4/Makefile
                 dune.pc])
AC_OUTPUT
