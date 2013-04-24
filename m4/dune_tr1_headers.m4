## -*- autoconf -*-
AC_DEFUN([DUNE_TR1_HEADERS], [
  # Allow disabling tr1 headers. E.g. needed for broken xlc on Blue Gene
  AC_LANG_PUSH([C++])
  AC_ARG_ENABLE([tr1-headers], 
    [AS_HELP_STRING([--disable-tr1-headers],
      [Prevents checking for  tr1 headers like tuple, array, etc. 
       This might be needed for broken compilers like xlc on Blue Gene])],
    [],
    [enable_tr1_headers=yes])
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
     AC_CACHE_CHECK([whether integral_constant conforming to C++11 is supported], dune_cv_integral_constant_cplusplus11, [
       AC_COMPILE_IFELSE([
         AC_LANG_PROGRAM([
           #include <type_traits>
           void f( int );
         ],[
           f( std::integral_constant< int, 42 >() );
         ])
       ],[
         dune_cv_integral_constant_cplusplus11=yes
       ],[
         dune_cv_integral_constant_cplusplus11=no
       ])
     ])
     AS_IF([test "x$dune_cv_integral_constant_cplusplus11" != "xno"],[
       AC_DEFINE([HAVE_INTEGRAL_CONSTANT], 1, [Define to 1 if std::integral_constant< T, v > is supported and casts into T])
     ])
     AC_CACHE_CHECK([whether std::hash from C++11 is supported], dune_cv_hash_cplusplus11, [
       AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM([[#include <functional>]],
               [[std::hash<int> hasher; hasher(42);]])],
         dune_cv_hash_cplusplus11=yes,
         dune_cv_hash_cplusplus11=no)
       ])
     AS_IF([test "x$dune_cv_hash_cplusplus11" != "xno"],
       [AC_DEFINE([HAVE_STD_HASH], 1, [Define to 1 if the std::hash template from C++11 is available])
     ])
     AC_CACHE_CHECK([whether std::tr1::hash from TR1 is supported], dune_cv_hash_cplusplustr1, [
       AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM([[#include <tr1/functional>]],
               [[std::tr1::hash<int> hasher; hasher(42);]])],
         dune_cv_hash_cplusplustr1=yes,
         dune_cv_hash_cplusplustr1=no)
       ])
     AS_IF([test "x$dune_cv_hash_cplusplustr1" != "xno"],
       [AC_DEFINE([HAVE_TR1_HASH], 1, [Define to 1 if the std::tr1::hash template from TR1 is available])
     ])
  ])
  AC_LANG_POP([C++])
])
