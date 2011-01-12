# tests compiler support for C++0x rvalue references
# the associated macro is called HAVE_RVALUE_REFERENCES

AC_DEFUN([RVALUE_REFERENCES_CHECK],[
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([GXX0X])
  AC_LANG_PUSH([C++])
  AC_MSG_CHECKING([whether rvalue references are supported])
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
      ])],[
    HAVE_RVALUE_REFERENCES=yes
    AC_MSG_RESULT(yes)], [
    HAVE_RVALUE_REFERENCES=no
    AC_MSG_RESULT(no)])
  if test "x$HAVE_RVALUe_REFERENCES" = xyes; then
    AC_DEFINE(HAVE_RVALUE_REFERENCES, 1, [Define to 1 if rvalue references are supported])
  fi
  AC_LANG_POP
])
