# tests for C++11 noexcept specifier support
# this test does not test the noexcept operator
# the associated macro is called HAVE_NOEXCEPT_SPECIFIER

AC_DEFUN([DUNE_CXX11_STD_DECLVAL_CHECK],[
  AC_CACHE_CHECK([for C++11 std::declval()],
    dune_cv_cxx11_std_declval,
    [
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([CXX11])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM(
        [
          #include <utility>

          template<typename T>
          struct check;

          template<>
          struct check<int&&>
          {
            int pass() { return 0; }
          };
        ],[
          return check<decltype(std::declval<int>())>().pass();
        ])],
      dune_cv_cxx11_std_declval=yes,
      dune_cv_cxx11_std_declval=no)
    AC_LANG_POP
  ])
  if test "$dune_cv_cxx11_std_declval" = yes; then
    AC_DEFINE(HAVE_STD_DECLVAL, 1,
      [Define to 1 if C++11 std::declval() is supported])
  fi
])
