# $Id$
# searches for albert-headers and libs

AC_DEFUN(DUNE_PATH_ALBERT,
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([DUNE_DIMENSION])

  AC_ARG_WITH(albert,
    AC_HELP_STRING([--with-albert=PATH],[directory with Albert inside]),
dnl expand tilde / other stuff
    eval with_albert=$with_albert
dnl extract absolute path
dnl eval with_albert=`cd $with_albert ; pwd`
    AC_SUBST(ALBERTROOT, $with_albert))

# also ask for elindex
  AC_ARG_WITH(albert_elindex,
            AC_HELP_STRING([--with-albert-elindex=INDEX],
	      [Albert element index (default is 1)]),,with_albert_elindex=1)
  AC_SUBST(ALBERT_ELINDEX, $with_albert_elindex)

# store old values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CFLAGS="$CFLAGS"
ac_save_LIBS="$LIBS"
LIBS=""

# is --with-albert=bla used?
if test x$with_albert != x ; then
  ALBERTROOT="$with_albert"
else
  # use some default value...
  ALBERTROOT="/usr/local/albert"
fi

LDFLAGS="-L$ALBERTROOT/lib"
CFLAGS="-I$ALBERTROOT/include"

# check for header
AC_CHECK_HEADER([albert.h], [ALBERT_INCLUDE="$CFLAGS"
	                     HAVE_ALBERT="1"],
  AC_MSG_WARN[albert.h not found in $ALBERTROOT/include])


# if header is found...
if test x$HAVE_ALBERT = x1 ; then
  # construct libname
  albertlibname=ALBERT$with_world_dim$with_problem_dim_0$with_albert_elindex
  AC_CHECK_LIB($albertlibname,[$LIBS="$LIBS -l$albertlibname"],
	[HAVE_ALBERT="0"
	AC_MSG_WARN($albertlibname not found!)])
fi

# still everything found?
if test x$HAVE_ALBERT = x1 ; then
  AC_CHECK_LIB(albert_util,[$LIBS="$LIBS -lalbert_util"],
	[HAVE_ALBERT="0"
	AC_MSG_WARN(libalbert_util not found!)])
fi

# survived all tests?
if test x$HAVE_ALBERT = x1 ; then
  AC_SUBST(ALBERT_LIBS, $LIBS)
  AC_SUBST(ALBERT_LDFLAGS, $LDFLAGS)
  AC_SUBST(ALBERT_CFLAGS, 
	"$CFLAGS -DEL_INDEX=$with_albert_elindex -DMYDIM=$with_problem_dim -DMYDOW=$with_world_dim")
  AC_DEFINE(HAVE_ALBERT, 1, [Define to 1 if albert-library is found])
fi
  
# also tell automake
AM_CONDITIONAL(ALBERT, test x$HAVE_ALBERT = x1)

# reset old values
LIBS="$ac_save_LIBS"
CFLAGS="$ac_save_CFLAGS"
LDFLAGS="$ac_save_LDFLAGS"

])