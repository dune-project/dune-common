# $Id$
# searches for alberta-headers and libs

# TODO:
#
# - alberta kann wahlweise GRAPE oder gltools
#   (http://www.wias-berlin.de/software/gltools/) verwenden, die sollten
#   vorher getestet werden
# - debug-Ziffer

AC_DEFUN([DUNE_PATH_ALBERTA],[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([DUNE_DIMENSION])

  AC_ARG_WITH(alberta,
    AC_HELP_STRING([--with-alberta=PATH],[directory with Alberta (Albert
    version 1.2 and higher) inside]))
# do not use alberta debug lib 
with_alberta_debug=0

# store old values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"
# LIBS=""

## do nothing if no --with-alberta was supplied
if test x$with_alberta != x && test x$with_alberta != xno ; then

  if test x$with_alberta == xyes ; then
    AC_MSG_ERROR([You have to provide a directory --with-alberta=PATH])
  fi

  if test -d $with_alberta; then
    # expand tilde / other stuff
    ALBERTAROOT=`cd $with_alberta && pwd`
  else
    AC_MSG_ERROR([Path $with_alberta supplied for --with-alberta does not exist!])
  fi

ALBERTA_LIB_PATH="$ALBERTAROOT/lib"
ALBERTA_INCLUDE_PATH="$ALBERTAROOT/include"

# Alberta needs special defined symbols

ALBERTADEF="-DDIM=$with_problem_dim -DDIM_OF_WORLD=$with_world_dim"

# set variables so that tests can use them
REM_CPPFLAGS=$CPPFLAGS

LDFLAGS="$LDFLAGS -L$ALBERTA_LIB_PATH"
CPPFLAGS="$CPPFLAGS $ALBERTADEF -DEL_INDEX=0 -I$ALBERTA_INCLUDE_PATH"

# check for header
AC_CHECK_HEADER([alberta.h], 
   [ALBERTA_CPPFLAGS="$ALBERTADEF -I$ALBERTA_INCLUDE_PATH"
	HAVE_ALBERTA="1"],
  AC_MSG_WARN([alberta.h not found in $ALBERTA_INCLUDE_PATH]))

CPPFLAGS="$REM_CPPFLAGS $ALBERTADEF -I$ALBERTA_INCLUDE_PATH"
REM_CPPFLAGS=

# if header is found...
if test x$HAVE_ALBERTA = x1 ; then
  AC_CHECK_LIB(alberta_util,[alberta_calloc],
	[ALBERTA_LIBS="-lalberta_util"
         ALBERTA_LDFLAGS="-L$ALBERTA_LIB_PATH"
         LIBS="$LIBS $ALBERTA_LIBS"],
	[HAVE_ALBERTA="0"
	AC_MSG_WARN(libalberta_util not found!)])
fi

# still everything found?
if test x$HAVE_ALBERTA = x1 ; then
  # construct libname
  # the zero is the sign of the no-debug-lib
  albertalibname="ALBERTA${with_problem_dim}${with_world_dim}_${with_alberta_debug}"
  AC_CHECK_LIB($albertalibname,[mesh_traverse],
	[ALBERTA_LIBS="-l$albertalibname $ALBERTA_LIBS"
   LIBS="$LIBS $ALBERTA_LIBS"],
	[HAVE_ALBERTA="0"
	AC_MSG_WARN(lib$albertalibname not found!)])
fi

## end of alberta check (--without wasn't set)
fi

# survived all tests?
if test x$HAVE_ALBERTA = x1 ; then
  AC_SUBST(ALBERTA_LIBS, $ALBERTA_LIBS)
  AC_SUBST(ALBERTA_LDFLAGS, $ALBERTA_LDFLAGS)
  AC_SUBST(ALBERTA_CPPFLAGS, $ALBERTA_CPPFLAGS)
  AC_DEFINE(HAVE_ALBERTA, 1, [Define to 1 if alberta-library is found])

  # add to global list
  DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $ALBERTA_LDFLAGS"
  DUNE_PKG_LIBS="$DUNE_PKG_LIBS $ALBERTA_LIBS"
  DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $ALBERTA_CPPFLAGS"

  # set variable for summary
  with_alberta="yes"
else
  AC_SUBST(ALBERTA_LIBS, "")
  AC_SUBST(ALBERTA_LDFLAGS, "")
  AC_SUBST(ALBERTA_CPPFLAGS, "")

  # set variable for summary
  with_alberta="no"
fi
  
# also tell automake
AM_CONDITIONAL(ALBERTA, test x$HAVE_ALBERTA = x1)

# reset old values
LIBS="$ac_save_LIBS"
CPPFLAGS="$ac_save_CPPFLAGS"
LDFLAGS="$ac_save_LDFLAGS"

])
