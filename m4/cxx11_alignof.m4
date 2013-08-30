# tests for C++11 alignof operator support
# the associated macro is called HAVE_ALIGNOF

AC_DEFUN([ALIGNOF_CHECK],[
  AC_CACHE_CHECK([for C++11 alignof], dune_cv_cxx11_alignof, [
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([GXX0X])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM([[
        ]],
        [[
          return !(alignof(double) > 0);
        ]])],
      [dune_cv_cxx11_alignof=yes],
      [dune_cv_cxx11_alignof=no])
    AC_LANG_POP
  ])
  if test "x$dune_cv_cxx11_alignof" = xyes; then
    AC_DEFINE(HAVE_ALIGNOF, 1, [Define to 1 if C++11 alignof is supported])
  fi
])
