#! /bin/bash
# $Id$
# Process this file with autoconf to produce a configure script.
AC_INIT(dune, 0.0, dune@hal.iwr.uni-heidelberg.de)
AM_INIT_AUTOMAKE(dune, 0.0, dune@hal.iwr.uni-heidelberg.de)
AC_CONFIG_SRCDIR([grid/common/grid.hh])
AM_CONFIG_HEADER([config.h])

# do almost nothing, an application using Dune should provide the
# checks (use the macro provided in m4/dune.m4 to do this)

# Checks for programs.
AC_PROG_CXX
AC_LANG_PUSH([C++])
DUNE_CHECK_COMPILER
DUNE_CHECKDEPRECATED

# don't build shared libs per default, this is way better for debugging...
AC_DISABLE_SHARED
AC_PROG_LIBTOOL

# check for auxiliary tools so that it's not fatal if they're missing
AC_CHECK_PROGS([DOXYGEN], [doxygen], [true])
AC_CHECK_PROGS([TEX], [latex], [true])
AC_CHECK_PROGS([BIBTEX], [bibtex], [true])
AC_CHECK_PROGS([DVIPDF], [dvipdf], [true])
AC_CHECK_PROGS([DVIPS], [dvips], [true])
AC_CHECK_PROGS([WML], [wml], [true])
DUNE_INKSCAPE

# special variable to include the documentation into the website
AC_ARG_WITH(duneweb,
    AC_HELP_STRING([--with-duneweb=PATH],[Only needed for website-generation, path to checked out version of dune-web]))
if test x$with_duneweb != x && test -e $with_duneweb/layout/default.wml ; then
  AC_MSG_WARN([duneweb-directory seems to be wrong!])
fi
if test -d $with_duneweb ; then
  with_duneweb=`(cd $with_duneweb && pwd)` ;
fi
AC_SUBST(DUNEWEBDIR, $with_duneweb)

echo
echo Checking for libraries the self-test could use:
echo

# -- check for other grid-libs so that they may be included into tests
# Alberta-check
DUNE_PATH_ALBERTA
# UG-check
DUNE_PATH_UG
DUNE_PATH_AMIRAMESH
# Yaspgrid-check
DUNE_MPI

# check for ALU3dGrid (needs MPI to build parallel version)
DUNE_PATH_ALU3DGRID

# check for ParMETIS
DUNE_PATH_PARMETIS

# how to create a deprecated warning
DUNE_CHECKDEPRECATED

# create symlink for consistent paths even when $(top_srcdir) is not
# called dune/ (if filesystem/OS supports symlinks)
AC_PROG_LN_S
if test x"$LN_S" == x"ln -s" ; then
  # Symlinks possible!

  # Note: we are currently in the build directory which may be != the
  # source directory

  # does a file already exist?
  if test -e dune ; then
    # exists: is a symlink?
    if test -L dune ; then
      if test -r dune/grid/common/grid.hh ; then
        # suppose that the link is ok and use it
        AM_CPPFLAGS='-I$(top_builddir)';
      else
        AC_MSG_ERROR([Symlink 'dune' exists but appears to be wrong! Please remove it manually])
      fi
    fi
  else
    echo Creating dune-symlink...
    # set symlink in build directory to sources
    ln -s $srcdir dune

    # sanity check
    if test -r dune/grid/common/grid.hh ; then
      AM_CPPFLAGS='-I$(top_builddir)';
    else
      AC_MSG_ERROR([Sanity check for symlink failed! Please send a bugreport to dune@hal.iwr.uni-heidelberg.de])
    fi
    # set proper include-path
    AM_CPPFLAGS='-I$(top_builddir)';
  fi
else
  # no symlinks possible... check name of directory

  # does dune-dir above exist and is it this directory?
  if test -d ../dune && test `pwd` == `( cd ../dune && pwd)` ; then
    DIR=`(cd .. && pwd)`;
    # use directory above current dir in include-path
    AM_CPPFLAGS="-I$DIR";
  else
    AC_MSG_ERROR([No symlinks supported! You have to rename the current directory to the name 'dune'])
  fi
fi

# preset variable to path such that #include <dune/...> works
AC_SUBST(AM_CPPFLAGS)

echo
echo Note: Most of the libraries checked for above are only used for the self-test
echo of Dune.  The library itself will build and the headers will work even if
echo Albert, MPI, etc. cannot be found.  An exception to this are UG and AmiraMesh
echo which need to be found right now if you want to use them later.
echo

# write output
AC_CONFIG_FILES([Makefile
	 bin/Makefile
	 common/Makefile
         common/test/Makefile
         fem/Makefile
         fem/common/Makefile
         fem/dgspace/Makefile
         fem/discfuncarray/Makefile
         fem/feop/Makefile
         fem/lagrangebase/Makefile
         fem/norms/Makefile
         fem/operator/Makefile
         fem/transfer/Makefile
         grid/Makefile
         grid/albertagrid/Makefile
         grid/alu3dgrid/Makefile
         grid/common/Makefile
	 grid/common/refinement/Makefile
         grid/onedgrid/Makefile
         grid/sgrid/Makefile
         grid/test/Makefile
         grid/uggrid/Makefile
         grid/yaspgrid/Makefile
         istl/Makefile
         istl/tutorial/Makefile
	 istl/test/Makefile
	 istl/paamg/Makefile
	 istl/paamg/test/Makefile
	 io/Makefile
	 io/file/Makefile
	 io/file/amiramesh/Makefile
	 io/file/rawdata/Makefile
	 io/visual/Makefile
	 io/visual/grape/Makefile
         lib/Makefile
         lib/test/Makefile
         operator/Makefile
         quadrature/Makefile
         quadrature/common/Makefile
         quadrature/barycenter/Makefile
         quadrature/gaussquadrature/Makefile
         quadrature/fixedorder/Makefile
         solver/Makefile
         solver/common/Makefile
         doc/Makefile
	 doc/devel/Makefile
	 doc/appl/Makefile
	 doc/appl/refelements/Makefile
	 doc/istl/Makefile
	 doc/istl/comm/Makefile
	 doc/layout/Makefile
	 doc/doxygen/Makefile
         m4/Makefile
	 am/Makefile
	 bin/wmlwrap
         dune.pc])
AC_OUTPUT
