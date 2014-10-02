# tests for C++11 noexcept specifier support
# this test does not test the noexcept operator
# the associated macro is called HAVE_NOEXCEPT_SPECIFIER

AC_DEFUN([DUNE_CXX11_NOEXCEPT_SPECIFIER_CHECK],[
  AC_CACHE_CHECK([for C++11 noexcept specifier],
    dune_cv_cxx11_noexcept_specifier_support,
    [
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([CXX11])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM(
        [
          void func1() noexcept {}

          void func2() noexcept(true) {}

          template <class T>
          void func3() noexcept(noexcept(T())) {}
        ],[
          func1();
          func2();
          func3<int>();
        ])],
      dune_cv_cxx11_noexcept_specifier_support=yes,
      dune_cv_cxx11_noexcept_specifier_support=no)
    AC_LANG_POP
  ])
  if test "$dune_cv_cxx11_noexcept_specifier_support" = yes; then
    AC_DEFINE(HAVE_NOEXCEPT_SPECIFIER, 1,
      [Define to 1 if C++11 nonexcept specifier is supported])
  fi
])
