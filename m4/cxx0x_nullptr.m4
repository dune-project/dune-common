AC_DEFUN([NULLPTR_CHECK],[
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([GXX0X])
  AC_LANG_PUSH([C++])
  AC_MSG_CHECKING([whether nullptr is supported])
  AC_TRY_COMPILE([],[
    char* ch = nullptr;
    ], [
    HAVE_NULLPTR=yes
    AC_MSG_RESULT(yes)], [
    HAVE_NULLPTR=no
    AC_MSG_RESULT(no)])
  if test "x$HAVE_NULLPTR" = xyes; then
    AC_DEFINE(HAVE_NULLPTR, 1, [Define to 1 if nullptr is supported])
  fi
  AC_LANG_POP
])
