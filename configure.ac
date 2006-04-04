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

# don't build shared libs per default, this is way better for debugging...
AC_DISABLE_SHARED
AC_PROG_LIBTOOL

DUNE_CHECK_COMPILER
DUNE_LINKCXX

# how to create a deprecated warning
DUNE_CHECKDEPRECATED

# check for auxiliary tools so that it's not fatal if they're missing
AC_CHECK_PROGS([DOXYGEN], [doxygen], [true])
AC_CHECK_PROGS([TEX], [latex], [true])
AC_CHECK_PROGS([BIBTEX], [bibtex], [true])
AC_CHECK_PROGS([DVIPDF], [dvipdf], [true])
AC_CHECK_PROGS([DVIPS], [dvips], [true])
AC_CHECK_PROGS([WML], [wml], [true])
AM_CONDITIONAL([WML], [test "x$WML" != xtrue])
AC_CHECK_PROGS([PERL], [perl], [true])
DUNE_INKSCAPE
AC_CHECK_PROGS([CONVERT], [convert], [true])

# doxygen and latex take a lot of time...
AC_ARG_ENABLE(documentation,
   AC_HELP_STRING([--disable-documentation],[don't generate docs, speeds up the build]))
AM_CONDITIONAL(BUILD_DOCS, test x$enable_documentation != xno)

# special variable to include the documentation into the website
AC_ARG_WITH(duneweb,
    AC_HELP_STRING([--with-duneweb=PATH],[Only needed for website-generation, path to checked out version of dune-web]))

if test x$with_duneweb != x ; then
   # parameter is set. Check it
   AC_MSG_CHECKING([whether passed Dune-Web directory appears correct])
   WEBTESTFILE="$with_duneweb/layout/default.wml"
   if test -d "$with_duneweb" && test -e "$WEBTESTFILE" ; then
      AC_MSG_RESULT([ok])
      # normalize path
      with_duneweb=`(cd $with_duneweb && pwd)` ;
   else
      if test -d "$with_duneweb" ; then
        AC_MSG_ERROR([$WEBTESTFILE not found in Dune-web dir $with_duneweb!])
      else
        AC_MSG_ERROR([Dune-Web directory $with_duneweb not found!])
      fi
   fi
fi
AC_SUBST(DUNEWEBDIR, $with_duneweb)

# special settings for check-log
AC_ARG_WITH(hostid,
  AC_HELP_STRING([--with-hostid=HOST_IDENTIFIER],
                 [host identifier used for automated test runs]))
if test "x$with_hostid" = "xno" ; then
  with_hostid="$ac_hostname ($(uname -sm), $COMPILER_NAME)";
fi
AC_SUBST(host, $with_hostid)
AC_ARG_WITH(tag,
  AC_HELP_STRING([--with-tag=TAG],
                 [tag to use for automated test runs]))
if test "x$with_tag" = "xno" ; then with_tag=foo; fi
AC_SUBST(tag, $with_tag)
AC_ARG_WITH(revision,
  AC_HELP_STRING([--with-revision=TAG],
                 [revision to use for automated test runs]))
if test "x$with_revision" = "xno" ; then with_revision=bar; fi
AC_SUBST(revision, $with_revision)

echo
echo Checking for libraries the self-test could use:
echo

# -- check for other grid-libs so that they may be included into tests
DUNE_PATH_GRAPE
# Alberta-check
DUNE_PATH_ALBERTA
# UG-check
DUNE_PATH_UG
# Check for the AmiraMesh library
DUNE_PATH_AMIRAMESH
# Check for the psurface library
DUNE_PATH_PSURFACE
# Yaspgrid-check
DUNE_MPI

# check for ALUGrid (needs MPI to build parallel version)
DUNE_PATH_ALUGRID

# check for Metis
IMMDX_LIB_METIS(,AC_MSG_WARN([METIS not found]))

# check for ParMETIS
DUNE_PATH_PARMETIS

# create symlink for consistent paths even when $(top_srcdir) is not
# called dune/ (if filesystem/OS supports symlinks)
AC_PROG_LN_S
if test x"$LN_S" = x"ln -s" ; then
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
  if test -d ../dune && test `pwd` = `( cd ../dune && pwd)` ; then
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
     doc/Makefile
     doc/devel/Makefile
     doc/appl/Makefile
     doc/appl/refelements/Makefile
     doc/istl/Makefile
     doc/istl/comm/Makefile
     doc/layout/Makefile
     doc/refinement/Makefile
     doc/doxygen/Makefile
	 disc/Makefile
	 disc/elasticity/Makefile
     disc/functions/Makefile
	 disc/groundwater/Makefile
	 disc/miscoperators/Makefile
     disc/operators/Makefile
	 disc/shapefunctions/Makefile
	 disc/shapefunctions/dgspace/Makefile
	 disc/shapefunctions/lagrange/Makefile
     disc/shapefunctions/test/Makefile
     grid/Makefile
     grid/albertagrid/Makefile
     grid/alu3dgrid/Makefile
     grid/common/Makefile
     grid/common/test/Makefile
	 grid/common/refinement/Makefile
     grid/onedgrid/Makefile
     grid/sgrid/Makefile
     grid/test/Makefile
     grid/uggrid/Makefile
	 grid/utility/Makefile
     grid/yaspgrid/Makefile
     fem/Makefile
     fem/norms/Makefile
     fem/transfer/Makefile
     istl/Makefile
     istl/tutorial/Makefile
	 istl/test/Makefile
	 istl/paamg/Makefile
	 istl/paamg/test/Makefile
	 io/Makefile
	 io/file/Makefile
     io/file/alberta/Makefile
	 io/file/amiramesh/Makefile
	 io/file/rawdata/Makefile
     io/file/test/Makefile
	 io/visual/Makefile
	 io/visual/grape/Makefile
     lib/Makefile
     quadrature/Makefile
     quadrature/gaussquadrature/Makefile
     quadrature/test/Makefile
     solver/Makefile
     solver/common/Makefile
     m4/Makefile
	 am/Makefile
	 bin/wmlwrap
	 bin/check-log-store
     dune.pc])
AC_OUTPUT

chmod +x bin/wmlwrap
chmod +x bin/check-log-store
