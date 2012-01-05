# tests for C++0x variadic template support
# the associated macro is called HAVE_VARIADIC_TEMPLATES

AC_DEFUN([VARIADIC_TEMPLATES_CHECK],[
  AC_CACHE_CHECK([whether variadic templates are supported], dune_cv_variadic_templates_support, [
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([GXX0X])
    AC_LANG_PUSH([C++])
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
        ])],
      dune_cv_variadic_templates_support=yes,
      dune_cv_variadic_templates_support=no)
    AC_LANG_POP
  ])  
  if test "x$dune_cv_variadic_templates_support" = xyes; then
    AC_DEFINE(HAVE_VARIADIC_TEMPLATES, 1, [Define to 1 if variadic templates are supported])
  fi
])
