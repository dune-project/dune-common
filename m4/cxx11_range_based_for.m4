# tests for C++11 range-based for support
# the associated define is called HAVE_RANGE_BASED_FOR

AC_DEFUN([DUNE_CXX11_RANGE_BASED_FOR],[
  AC_CACHE_CHECK([for C++11 range-based for], [dune_cv_cxx11_range_based_for_support], [
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([CXX11])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM(,[[
          int arr[3];
          for(int &val : arr)
            val = 0;
        ]])],
      [dune_cv_cxx11_range_based_for_support=yes],
      [dune_cv_cxx11_range_based_for_support=no])
    AC_LANG_POP
  ])
  if test "x$dune_cv_cxx11_range_based_for_support" = xyes; then
    AC_DEFINE([HAVE_RANGE_BASED_FOR], [1], [Define to 1 if C++11 range-based for is supported])
  fi
])
