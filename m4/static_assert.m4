AC_DEFUN([GXX0X],[
  AC_REQUIRE([AC_PROG_CXX])
  AC_LANG_PUSH([C++])
  if test "x$GXX" = xyes; then
    AC_MSG_CHECKING([whether g++ accepts -std=c++0x])
    ac_save_CXXFLAGS="$CXXFLAGS"
    CXXFLAGS="$CXXFLAGS -std=c++0x"
    AC_TRY_COMPILE([],[],[
      AC_MSG_RESULT(yes)], [
      CXXFLAGS="$ac_save_CXXFLAGS"
      AC_MSG_RESULT(no)])
  fi
  AC_LANG_POP
])

AC_DEFUN([STATIC_ASSERT_CHECK],[
  AC_LANG_PUSH([C++])
  AC_MSG_CHECKING([whether static_assert is supported])
  AC_TRY_COMPILE([],[static_assert(true,"MSG")], [
    HAVE_STATIC_ASSERT=0,
    AC_MSG_RESULT(yes)], [
    HAVE_STATIC_ASSERT=1,
    AC_MSG_RESULT(no)])
  if test x$HAVE_STATIC_ASSERT = x1; then
    AC_DEFINE(HAVE_STATIC_ASSERT, 1, [Define to 1 if static_assert is supported])
  fi
  AC_LANG_POP
])
