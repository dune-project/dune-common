# $Id$
# searches for albert-headers and libs

AC_DEFUN(AM_PATH_ALBERT,
[
  AC_REQUIRE([AC_PROG_CC])

  AC_ARG_WITH(albert,
    AC_HELP_STRING([--with-albert=PATH],[directory with Albert inside]),
dnl expand tilde / other stuff
    eval with_albert=$with_albert
dnl extract absolute path
dnl eval with_albert=`cd $with_albert ; pwd`
    AC_SUBST(ALBERTROOT, $with_albert))

dnl is --with-albert=bla used?
  if test x$with_albert != x ; then
  AC_CHECK_FILE($ALBERTROOT/include/albert.h,
    AC_SUBST(ALBERT_INCLUDE, $ALBERTROOT/include/),
    AC_MSG_ERROR([$ALBERTROOT does not contain Albert-Headers!])
  )
  AC_CHECK_FILE($ALBERTROOT/lib/libalbert_util.a,
    AC_SUBST(ALBERT_LIBS, -L$ALBERTROOT/lib/),
    AC_MSG_ERROR([$ALBERTROOT does not contain the compiled libs...])
  )
fi
  
])