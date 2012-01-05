# tests whether the compiler supports SFINAE on variadic template constructors
# within template classes. GCC 4.3 fails this test.
# the associated macro is called HAVE_VARIADIC_CONSTRUCTOR_SFINAE

AC_DEFUN([VARIADIC_CONSTRUCTOR_SFINAE_CHECK],[
  AC_CACHE_CHECK([whether SFINAE on variadic template constructors is fully supported], 
    dune_cv_variadic_constructor_sfinae_support, [
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([GXX0X])
    AC_LANG_PUSH([C++])
    AC_RUN_IFELSE([
      AC_LANG_PROGRAM([#include <cassert>
        #include <functional>

        template<typename... U>
        struct A
        {

          template<typename... T,
                   typename = typename std::enable_if<(sizeof...(T) < 2)>::type
                   >
          A(T... t)
            : i(1)
          {}

          template<typename... T,
                   typename = typename std::enable_if<(sizeof...(T) >= 2)>::type,
                   typename = void
                   >
          A(T... t)
            : i(-1)
          {}

          A()
            : i(1)
          {}

          int i;
        };],
        [
          assert( A<int>().i +
                  A<int>(2).i +
                  A<int>("foo",3.4).i +
                  A<int>(8,'a',A<int>()).i == 0);
          return 0;
        ])],
      dune_cv_variadic_constructor_sfinae_support=yes,
      dune_cv_variadic_constructor_sfinae_support=no)
    AC_LANG_POP
  ])
  if test "x$dune_cv_variadic_constructor_sfinae_support" = xyes; then
    AC_DEFINE(HAVE_VARIADIC_CONSTRUCTOR_SFINAE, 1, [Define to 1 if SFINAE on variadic template constructors is fully supported])
  fi
])
