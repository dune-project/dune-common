## -*- autoconf -*-
AC_DEFUN([DUNE_TR1_HEADERS], [
  # Allow disabling tr1 headers. E.g. needed for broken xlc on Blue Gene
  AC_LANG_PUSH([C++])
  AC_ARG_ENABLE([tr1-headers], 
    [AS_HELP_STRING([--disable-tr1-headers],
      [Prevents checking for  tr1 headers like tuple, array, etc. 
       This might be needed for broken compilers like xlc on Blue Gene])],
    [],
    [enable_tr_headers=yes])
  AS_IF([test "x$enable_tr1_headers" != "xno"],
    [AC_CHECK_HEADERS([type_traits tr1/type_traits array tr1/array tuple tr1/tuple])])
  AC_LANG_POP([C++])
])
