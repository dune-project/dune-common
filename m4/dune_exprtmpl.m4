AC_DEFUN([DUNE_EXPRTMPL],[
  # enable experimental features
  AC_ARG_ENABLE(expressiontemplates,
   AS_HELP_STRING([--enable-expressiontemplates],[enable experimental expression templates in dune]))
  AS_IF([test "x$enable_expressiontemplates" = "xyes"],[
    AC_DEFINE([DUNE_EXPRESSIONTEMPLATES], [1], 
      [Define to 1 if the experimental expression templates should be used])])
  AM_CONDITIONAL([EXPRESSIONTEMPLATES],
    [test "x$enable_expressiontemplates" = "xyes"])
])
