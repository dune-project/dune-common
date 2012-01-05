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
    [AC_CHECK_HEADERS([type_traits tr1/type_traits tuple tr1/tuple])
     AC_CACHE_CHECK([whether <array> C++0x is supported], dune_cv_array_cplusplus0x, [
       AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM([[#include <array>]],
               [[std::array<int,2> a; a.fill(9);]])],
         dune_cv_array_cplusplus0x=yes,
         dune_cv_array_cplusplus0x=no)
       ])
     AS_IF([test "x$dune_cv_array_cplusplus0x" != "xno"],
       [AC_DEFINE([HAVE_ARRAY], 1, [Define to 1 if the <array> C++0x is available and support array::fill])
     ])
  ])
  AC_LANG_POP([C++])
])
