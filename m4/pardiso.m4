AC_DEFUN([DUNE_PARDISO], [
AC_PREREQ(2.50)
AC_REQUIRE([AC_F77_LIBRARY_LDFLAGS])
AC_REQUIRE([ACX_LAPACK])
acx_pardiso_ok=no

AC_ARG_WITH(pardiso,
    [AC_HELP_STRING([--with-pardiso=<lib>], [use PARDISO library <lib>])])
case $with_pardiso in
    yes | "") ;;
    no) acx_pardiso_ok=disable ;;
    -* | */* | *.a | *.so | *.so.* | *.o) PARDISO_LIBS="$with_pardiso" ;;
    *) PARDISO_LIBS="-l$with_pardiso" ;;
esac
# Get fortran linker names of PARDISO functions to check for.
AC_F77_FUNC(pardisoinit)
acx_pardiso_save_LIBS="$LIBS"
LIBS="$LAPACK_LIBS $BLAS_LIBS $LIBS $FLIBS"

# First, check PARDISO_LIBS environment variable
if test $acx_pardiso_ok = no; then
if test "x$PARDISO_LIBS" != x; then
    save_LIBS="$LIBS"; LIBS="$PARDISO_LIBS $LIBS"
    AC_MSG_CHECKING([for $pardisoinit in $PARDISO_LIBS])
    AC_TRY_LINK_FUNC($pardisoinit, [dnl
	# add to global list
	DUNE_PKG_LIBS="$DUNE_PKG_LIBS $PARDISO_LIBS $LAPACK_LIBS $BLAS_LIBS $LIBS $FLIBS"
	acx_pardiso_ok=yes], 
	[PARDISO_LIBS=""])
    AC_MSG_RESULT($acx_pardiso_ok)
    LIBS="$save_LIBS"
fi
fi

# tell automake	
AM_CONDITIONAL(PARDISO, test $acx_pardiso_ok = yes)

AC_SUBST(PARDISO_LIBS)

LIBS="$acx_pardiso_save_LIBS"

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$acx_pardiso_ok" = xyes; then
        ifelse([$1],,AC_DEFINE(HAVE_PARDISO,1,[Define if you have a
PARDISO library.]),[$1])
        :
else
        acx_pardiso_ok=no
        $2
fi
])dnl SET_PARDISO

