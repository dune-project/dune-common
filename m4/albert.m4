# $Id$
# searches for albert-headers and libs

# TODO:
#
# - albert kann wahlweise GRAPE oder gltools
#   (http://www.wias-berlin.de/software/gltools/) verwenden, die sollten
#   vorher getestet werden
# - debug-Ziffer

# EL_INDEX=1: the element indices should be precomputed instead of
# generated on the fly => default value

AC_DEFUN([DUNE_PATH_ALBERT],[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([DUNE_DIMENSION])

  AC_ARG_WITH(albert,
    AC_HELP_STRING([--with-albert=PATH],[directory with Albert inside]))

# also ask for elindex
  AC_ARG_WITH(albert_elindex,AC_HELP_STRING([--with-albert-elindex=INDEX],[Albert element index (default is 1)]),,with_albert_elindex=1)
  AC_SUBST(ALBERT_ELINDEX, $with_albert_elindex)

# store old values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"
# LIBS=""

# is --with-albert=bla used?
if test x$with_albert != x ; then
    if test -d $with_albert; then
      # expand tilde / other stuff
      ALBERTROOT=`cd $with_albert && pwd`
    else
      AC_MSG_ERROR([directory $with_albert does not exist])
    fi      
else
  # use some default value...
  ALBERTROOT="/usr/local/albert"
fi

ALBERT_LIB_PATH="$ALBERTROOT/lib"
ALBERT_INCLUDE_PATH="$ALBERTROOT/include"

# Albert needs special defined symbols

ALBERTDEF="-DDIM=$with_problem_dim -DDIM_OF_WORLD=$with_world_dim -DEL_INDEX=$with_albert_elindex"

# set variables so that tests can use them
LDFLAGS="$LDFLAGS -L$ALBERT_LIB_PATH"
CPPFLAGS="$CPPFLAGS $ALBERTDEF -I$ALBERT_INCLUDE_PATH"

# check for header
AC_CHECK_HEADER([albert.h], 
   [ALBERT_CPPFLAGS="$ALBERTDEF -I$ALBERT_INCLUDE_PATH"
	HAVE_ALBERT="1"],
  AC_MSG_WARN([albert.h not found in $ALBERT_INCLUDE_PATH]))

# if header is found...
if test x$HAVE_ALBERT = x1 ; then
  AC_CHECK_LIB(albert_util,[albert_calloc],
	[ALBERT_LIBS="-lalbert_util"
         ALBERT_LDFLAGS="-L$ALBERT_LIB_PATH"
         LIBS="$LIBS $ALBERT_LIBS"],
	[HAVE_ALBERT="0"
	AC_MSG_WARN(libalbert_util not found!)])
fi

# still everything found?
if test x$HAVE_ALBERT = x1 ; then
  # construct libname
  # the zero is the sign of the no-debug-lib
  albertlibname="ALBERT${with_problem_dim}${with_world_dim}_0${with_albert_elindex}"
  AC_CHECK_LIB($albertlibname,[mesh_traverse],
	[ALBERT_LIBS="-l$albertlibname $ALBERT_LIBS"
   LIBS="$LIBS $ALBERT_LIBS"],
	[HAVE_ALBERT="0"
	AC_MSG_WARN(lib$albertlibname not found!)])
fi


# survived all tests?
if test x$HAVE_ALBERT = x1 ; then
  AC_SUBST(ALBERT_LIBS, $ALBERT_LIBS)
  AC_SUBST(ALBERT_LDFLAGS, $ALBERT_LDFLAGS)
  AC_SUBST(ALBERT_CPPFLAGS, $ALBERT_CPPFLAGS)
  AC_DEFINE(HAVE_ALBERT, 1, [Define to 1 if albert-library is found])

  # add to global list
  DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $ALBERT_LDFLAGS"
  DUNE_PKG_LIBS="$DUNE_PKG_LIBS $ALBERT_LIBS"
  DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $ALBERT_CPPFLAGS"
else
  echo no albert found...
  AC_SUBST(ALBERT_LIBS, "")
  AC_SUBST(ALBERT_LDFLAGS, "")
  AC_SUBST(ALBERT_CPPFLAGS, "")
fi
  
# also tell automake
AM_CONDITIONAL(ALBERT, test x$HAVE_ALBERT = x1)

# reset old values
LIBS="$ac_save_LIBS"
CPPFLAGS="$ac_save_CPPFLAGS"
LDFLAGS="$ac_save_LDFLAGS"

])
