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

# check for auxiliary tools so that it's not fatal if they're missing
AC_CHECK_PROGS([DOXYGEN], [doxygen], [true])
AC_CHECK_PROGS([TEX], [latex], [true])
AC_CHECK_PROGS([BIBTEX], [bibtex], [true])
AC_CHECK_PROGS([DVIPDF], [dvipdf], [true])
AC_CHECK_PROGS([DVIPS], [dvips], [true])

# create symlink for consistent paths even when $(top_srcdir) is not
# called dune/ (if filesystem/OS supports symlinks)
AC_PROG_LN_S
if test x"$LN_S" == x"ln -s" ; then
  # Symlinks possible!

  # does a file already exist?
  if test -e dune ; then
    # exists: is a symlink?
    if test -L dune ; then
      if test -e dune/grid/common/grid.hh ; then
        # suppose that the link is ok and use it
        AM_CPPFLAGS='-I$(top_srcdir)';
      else
        AC_MSG_ERROR([Symlink 'dune' exists but appears to be wrong! Please remove it manually])
      fi
    fi
  else
    echo Creating dune-symlink...
    # set symlink
    ln -s . dune ;
    # set proper include-path
    AM_CPPFLAGS='-I$(top_srcdir)';
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

# write output
AC_CONFIG_FILES([Makefile
	         common/Makefile
		 common/test/Makefile
                 fem/Makefile
                 fem/common/Makefile
                 fem/discfuncarray/Makefile
                 fem/operator/Makefile
                 grid/Makefile
                 grid/albertgrid/Makefile
                 grid/common/Makefile
                 grid/sgrid/Makefile
                 grid/yaspgrid/Makefile
		 istl/Makefile
		 istl/doc/Makefile
		 istl/tutorial/Makefile
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
                 solver/pmgsolver/Makefile
		 doc/Makefile
		 m4/Makefile
                 dune.pc])
AC_OUTPUT
