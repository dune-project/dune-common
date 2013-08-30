# tests for C++11 conditional support
# the associated macro is called HAVE_STD_CONDITIONAL

AC_DEFUN([CXX11_CONDITIONAL_CHECK],[
  AC_CACHE_CHECK([for C++11 std::conditional], dune_cv_cxx11_conditional_support, [
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([GXX0X])
    AC_LANG_PUSH([C++])
    AC_RUN_IFELSE([
      AC_LANG_PROGRAM([

        #include <type_traits>

        ],
        [
          return std::conditional<true,std::integral_constant<int,0>,void>::type::value;
        ])],
      dune_cv_cxx11_conditional_support=yes,
      dune_cv_cxx11_conditional_support=no)
    AC_LANG_POP
  ])
  if test "x$dune_cv_cxx11_conditional_support" = xyes; then
    AC_DEFINE(HAVE_STD_CONDITIONAL, 1, [Define to 1 if C++11 std::conditional is supported])
  fi
])
