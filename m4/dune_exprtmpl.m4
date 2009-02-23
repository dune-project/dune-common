AC_DEFUN([DUNE_EXPRTMPL],[
  # enable experimental features
  AC_ARG_ENABLE(expressiontemplates,
   AC_HELP_STRING([--enable-expressiontemplates],[enable experimental expressiontemplates in dune]))
  if test x$enable_expressiontemplates = xyes; then
    AC_DEFINE([DUNE_EXPRESSIONTEMPLATES], [1], 
      [Define to 1 if the experimental expression templates should be used])
  fi
  AM_CONDITIONAL([EXPRESSIONTEMPLATES],
    [test x$enable_expressiontemplates = xyes])
])