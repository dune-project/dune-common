# Check for inkscape and define automake conditional

AC_DEFUN([DUNE_INKSCAPE],[
	AC_CHECK_PROG([INKSCAPE], [inkscape], [inkscape], [no])
	AC_ARG_VAR([INKSCAPE], [path to inkscape to regenerate .png's from .svg's.])
	AM_CONDITIONAL([INKSCAPE], [test "x$INKSCAPE" != xfalse && test "x$INKSCAPE" != xno])
])
