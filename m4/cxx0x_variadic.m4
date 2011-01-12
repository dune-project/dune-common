# tests for C++0x variadic template support
# the associated macro is called HAVE_VARIADIC_TEMPLATES

AC_DEFUN([VARIADIC_TEMPLATES_CHECK],[
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([GXX0X])
  AC_LANG_PUSH([C++])
  AC_MSG_CHECKING([whether variadic templates are supported])
  AC_RUN_IFELSE([
    AC_LANG_PROGRAM([#include<cassert>

      template<typename... T>
      int addints(T... x);

      int add_ints()
      {
        return 0;
      }

      template<typename T1, typename... T>
      int add_ints(T1 t1, T... t)
      {
        return t1 + add_ints(t...);
      }], 
      [
        assert( 5 == add_ints(9,3,-5,-2) );
        return 0;
      ])],[
    HAVE_VARIADIC_TEMPLATES=yes
    AC_MSG_RESULT(yes)], [
    HAVE_VARIADIC_TEMPLATES=no
    AC_MSG_RESULT(no)])
  if test "x$HAVE_VARIADIC_TEMPLATES" = xyes; then
    AC_DEFINE(HAVE_VARIADIC_TEMPLATES, 1, [Define to 1 if variadic templates are supported])
  fi
  AC_LANG_POP
])
