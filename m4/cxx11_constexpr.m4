# tests for C++11 constexpr support
# the associated macro is called HAVE_CONSTEXPR

AC_DEFUN([CXX11_CONSTEXPR_CHECK],[
  AC_CACHE_CHECK([for C++11 constexpr], dune_cv_cxx11_constexpr_support, [
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([GXX0X])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM([
          constexpr int foo () { return 0; }

          template< int v >
          struct A
          {
            static const int value = v;
          };
        ],[
          return A< foo() >::value;
        ])],
      dune_cv_cxx11_constexpr_support=yes,
      dune_cv_cxx11_constexpr_support=no)
    AC_LANG_POP
  ])
  if test "x$dune_cv_cxx11_constexpr_support" = xyes; then
    AC_DEFINE(HAVE_CONSTEXPR, 1, [Define to 1 if C++11 constexpr is supported])
  fi
])
