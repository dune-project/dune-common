# tests for C++11 initializer list support
# the associated macro is called HAVE_INITIALIZER_LIST

AC_DEFUN([INITIALIZER_LIST_CHECK],[
  AC_CACHE_CHECK([whether std::initializer_list is supported], dune_cv_initializer_list_support, [
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([GXX0X])
    AC_LANG_PUSH([C++])
    AC_RUN_IFELSE([
      AC_LANG_PROGRAM([

        #include <initializer_list>
        #include <vector>

        struct A
        {

          A(std::initializer_list<int> il)
            : vec(il)
          {}

          std::vector<int> vec;
        };

        ],
        [
          A a{1,3,4,5};
          return 0;
        ])],
      dune_cv_initializer_list_support=yes,
      dune_cv_initializer_list_support=no)
    AC_LANG_POP
  ])
  if test "x$dune_cv_initializer_list_support" = xyes; then
    AC_DEFINE(HAVE_INITIALIZER_LIST, 1, [Define to 1 if std::initializer_list is supported])
  fi
])
