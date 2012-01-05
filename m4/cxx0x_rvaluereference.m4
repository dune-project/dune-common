# tests compiler support for C++0x rvalue references
# the associated macro is called HAVE_RVALUE_REFERENCES

AC_DEFUN([RVALUE_REFERENCES_CHECK],[
  AC_CACHE_CHECK([whether rvalue references are supported], dune_cv_rvalue_references_support, [
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([GXX0X])
    AC_LANG_PUSH([C++])
    AC_RUN_IFELSE([
      AC_LANG_PROGRAM([#include<cassert>
        #include <utility>
        int foo(int&& x) { return 1; }
        int foo(const int& x) { return -1; }

        template<typename T>
        int forward(T&& x)
        {
            return foo(std::forward<T>(x));
        }], 
        [
          int i = 0;
          assert( forward(i) + forward(int(2)) == 0);
          return 0;
        ])],
      dune_cv_rvalue_references_support=yes,
      dune_cv_rvalue_references_support=no)
      AC_LANG_POP
  ])
  if test "x$dune_cv_rvalue_references_support" = xyes; then
    AC_DEFINE(HAVE_RVALUE_REFERENCES, 1, [Define to 1 if rvalue references are supported])
  fi
])
