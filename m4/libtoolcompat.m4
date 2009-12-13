dnl Compatibility m4 to make dune work with
dnl libtool 1.5 and libtool 2.x

AC_DEFUN([LT_COMPAT],[
  # LT_COMPAT
  m4_ifdef([LT_OUTPUT],[],[AC_DEFUN([LT_OUTPUT])])
])
