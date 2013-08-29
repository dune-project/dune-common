# tests for C++11 lambda expression support
# the associated macro is called HAVE_LAMBDA_EXPR

AC_DEFUN([LAMBDA_EXPR_CHECK],[
  AC_CACHE_CHECK([whether C++11 lambda expressions are supported], dune_cv_cxx11_lambda_expr, [
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([GXX0X])
    AC_LANG_PUSH([C++])
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM([[
          #include <algorithm>
        ]],
        [[
          int x[10] = {5};
          int res = 0;
          std::for_each(x,x+10,[&res](int p){ res += p; });
          return 0;
        ]])],
      [dune_cv_cxx11_lambda_expr=yes],
      [dune_cv_cxx11_lambda_expr=no])
    AC_LANG_POP
  ])
  if test "x$dune_cv_cxx11_lambda_expr" = xyes; then
    AC_DEFINE(HAVE_LAMBDA_EXPR, 1, [Define to 1 if C++11 lambda expressions are supported])
  fi
])
