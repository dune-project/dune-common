AC_DEFUN([STATIC_ASSERT_CHECK],[
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([GXX0X])
  AC_LANG_PUSH([C++])
  AC_MSG_CHECKING([whether static_assert is supported])
  AC_TRY_COMPILE([],[static_assert(true,"MSG")], [
    HAVE_STATIC_ASSERT=yes
    AC_MSG_RESULT(yes)], [
    HAVE_STATIC_ASSERT=no
    AC_MSG_RESULT(no)])
  if test "x$HAVE_STATIC_ASSERT" = xyes; then
    AC_DEFINE(HAVE_STATIC_ASSERT, 1, [Define to 1 if static_assert is supported])
  fi
  AC_LANG_POP
])
