# tests whether the compiler supports SFINAE on variadic template constructors
# within template classes. GCC 4.3 fails this test.
# the associated macro is called HAVE_VARIADIC_CONSTRUCTOR_SFINAE

AC_DEFUN([VARIADIC_CONSTRUCTOR_SFINAE_CHECK],[
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([GXX0X])
  AC_LANG_PUSH([C++])
  AC_MSG_CHECKING([whether SFINAE on variadic template constructors is fully supported])
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
      ])],[
    HAVE_VARIADIC_CONSTRUCTOR_SFINAE=yes
    AC_MSG_RESULT(yes)], [
    HAVE_VARIADIC_CONSTRUCTOR_SFINAE=no
    AC_MSG_RESULT(no)])
  if test "x$HAVE_VARIADIC_CONSTRUCTOR_SFINAE" = xyes; then
    AC_DEFINE(HAVE_VARIADIC_CONSTRUCTOR_SFINAE, 1, [Define to 1 if SFINAE on variadic template constructors is fully supported])
  fi
  AC_LANG_POP
])
