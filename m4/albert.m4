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

AC_DEFUN(DUNE_PATH_ALBERT,
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([DUNE_DIMENSION])

  AC_ARG_WITH(albert,
    AC_HELP_STRING([--with-albert=PATH],[directory with Albert inside]),
dnl expand tilde / other stuff
    eval with_albert=$with_albert)
dnl extract absolute path
dnl eval with_albert=`cd $with_albert ; pwd`

# also ask for elindex
  AC_ARG_WITH(albert_elindex,
            AC_HELP_STRING([--with-albert-elindex=INDEX],
	      [Albert element index (default is 1)]),,with_albert_elindex=1)
  AC_SUBST(ALBERT_ELINDEX, $with_albert_elindex)

# store old values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"
# LIBS=""

# is --with-albert=bla used?
if test x$with_albert != x ; then
  ALBERTROOT="$with_albert"
else
  # use some default value...
  ALBERTROOT="/usr/local/albert"
fi

# Albert needs special defined symbols

ALBERTDEF="-DDIM_OF_WORLD=$with_world_dim -DDIM=$with_problem_dim -DEL_INDEX=$with_albert_elindex"

# set variables so that tests can use them
LDFLAGS="$LDFLAGS -L$ALBERTROOT/lib"
CPPFLAGS="$CPPFLAGS $ALBERTDEF -I$ALBERTROOT/include"

# check for header
AC_CHECK_HEADER([albert.h], 
   [ALBERT_CPPFLAGS="$ALBERTDEF -I$ALBERTROOT/include"
	HAVE_ALBERT="1"],
  AC_MSG_WARN([albert.h not found in $ALBERTROOT]))

# if header is found...
if test x$HAVE_ALBERT = x1 ; then
  AC_CHECK_LIB(albert_util,[albert_calloc],
	[ALBERT_LIBS="-lalbert_util"
         ALBERT_LDFLAGS="-L$ALBERTROOT/lib"
         LIBS="$LIBS $ALBERT_LIBS"],
	[HAVE_ALBERT="0"
	AC_MSG_WARN(libalbert_util not found!)])
fi

# still everything found?
if test x$HAVE_ALBERT = x1 ; then
  # construct libname
  # the zero is the sign of the no-debug-lib
  albertlibname="ALBERT${with_world_dim}${with_problem_dim}_0${with_albert_elindex}"
  AC_CHECK_LIB($albertlibname,[get_dof_index],
	[ALBERT_LIBS="$ALBERT_LIBS -l$albertlibname"],
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