# $Id$
# searches for albert-headers and libs

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
ac_save_CFLAGS="$CFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_CXXFLAGS="$CXXFLAGS"
ac_save_LIBS="$LIBS"
LIBS=""

# is --with-albert=bla used?
if test x$with_albert != x ; then
  ALBERTROOT="$with_albert"
else
  # use some default value...
  ALBERTROOT="/usr/local/albert"
fi

LDFLAGS="$LDFLAGS -L$ALBERTROOT/lib"
CPPFLAGS="$CPPFLAGS -I$ALBERTROOT/include"

# check for header
AC_CHECK_HEADER([albert.h], 
   [ALBERT_CFLAGS="-I$ALBERTROOT/include"
	HAVE_ALBERT="1"],
  AC_MSG_WARN([albert.h not found in $ALBERTROOT]))

# if header is found...
if test x$HAVE_ALBERT = x1 ; then
  # construct libname
  albertlibname="ALBERT${with_world_dim}${with_problem_dim}_0${with_albert_elindex}"
  AC_CHECK_LIB($albertlibname,[albert],
	[$ALBERT_LIBS="-l$albertlibname"],
	[HAVE_ALBERT="0"
	AC_MSG_WARN(lib$albertlibname not found!)])
fi

# still everything found?
if test x$HAVE_ALBERT = x1 ; then
  AC_CHECK_LIB(albert_util,[albert_util],
	[$ALBERT_LIBS="$ALBERT_LIBS -lalbert_util"],
	[HAVE_ALBERT="0"
	AC_MSG_WARN(libalbert_util not found!)])
fi

# survived all tests?
if test x$HAVE_ALBERT = x1 ; then
  AC_SUBST(ALBERT_LIBS, $ALBERT_LIBS)
  AC_SUBST(ALBERT_LDFLAGS, $LDFLAGS)
  AC_SUBST(ALBERT_CFLAGS, 
	"$ALBERT_CFLAGS -DEL_INDEX=$with_albert_elindex -DMYDIM=$with_problem_dim -DMYDOW=$with_world_dim")
  AC_DEFINE(HAVE_ALBERT, 1, [Define to 1 if albert-library is found])
else
  echo no albert found...
  AC_SUBST(ALBERT_LIBS, "")
  AC_SUBST(ALBERT_LDFLAGS, "")
  AC_SUBST(ALBERT_CFLAGS, "")
fi
  
# also tell automake
AM_CONDITIONAL(ALBERT, test x$HAVE_ALBERT = x1)

# reset old values
LIBS="$ac_save_LIBS"
CFLAGS="$ac_save_CFLAGS"
CPPFLAGS="$ac_save_CPPFLAGS"
CXXFLAGS="$ac_save_CXXFLAGS"
LDFLAGS="$ac_save_LDFLAGS"

])