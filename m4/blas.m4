# $Id$
# searches for blas header and libs

AC_DEFUN([DUNE_PATH_BLAS],[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([DUNE_DIMENSION])

  AC_ARG_ENABLE(blas,
   AC_HELP_STRING([--enable-blas], [enable use of BLAS Library (default is disabled)]),
    [case "$enableval" in
      yes)
        AC_SUBST(HAVE_BLAS, "1")
        m4_bpatsubst([blas],-,_)_ENABLE=yes
        ;;
      no)
        AC_SUBST(HAVE_BLAS, "0")
        m4_bpatsubst([blas],-,_)_ENABLE=yes
      ;;
    *)
     ;;
   esac])

  AC_ARG_WITH(blas,
    AC_HELP_STRING([--with-blas-lib=PATH],[directory with BLAS Library inside (default=/usr/lib)]) )

# also ask for elindex

# store old values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_LIBS="$LIBS"
LIBS=""

# is --with-blas=bla used?
if test x$with_blas_lib != x ; then
    if test -d $with_blas; then
      # expand tilde / other stuff
      BLASROOT=`cd $with_blas_lib && pwd`
    else
      AC_MSG_ERROR([directory $with_blas does not exist])
    fi      
else
  # use some default value...
  BLASROOT="/usr/lib"
fi

BLAS_LIB_PATH="$BLASROOT"

# set variables so that tests can use them
LDFLAGS="$LDFLAGS -L$BLAS_LIB_PATH"

# no header check for blas, because no headers found

if test x$HAVE_BLAS = x1 ; then
  AC_LANG_PUSH(Fortran 77)
  AC_CHECK_LIB(blas, [dnrm2],
               [BLAS_LIBS="-lblas"
                BLAS_LDFLAGS="-L$BLAS_LIB_PATH"
                AC_DEFINE(HAVE_LIBBLAS, 1, [Define to 1 if you have libblas])],
               [AC_MSG_ERROR([Required BLAS library was not found])])
  AC_LANG_POP(Fortran 77)
fi

# survived all tests?
if test x$HAVE_BLAS = x1 ; then
  echo "*****  BLAS Library found in $BLAS_LIB_PATH"
  AC_SUBST(BLAS_LIBS, $BLAS_LIBS)
  AC_SUBST(BLAS_LDFLAGS, $BLAS_LDFLAGS)
  AC_SUBST(BLAS_CPPFLAGS, $BLAS_CPPFLAGS)
  AC_DEFINE(HAVE_BLAS, 1, [Define to 1 if blas-library is found])

  # add to global list
  DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $BLAS_LDFLAGS"
  DUNE_PKG_LIBS="$DUNE_PKG_LIBS $BLAS_LIBS"
else
  echo "*****  no BLAS Library is uesd. Enable with the --enable-blas option." 
  AC_SUBST(BLAS_LIBS, "")
  AC_SUBST(BLAS_LDFLAGS, "")
fi
  
# also tell automake
AM_CONDITIONAL(BLAS, test x$HAVE_BLAS = x1)

# reset old values
LIBS="$ac_save_LIBS"
LDFLAGS="$ac_save_LDFLAGS"

])
