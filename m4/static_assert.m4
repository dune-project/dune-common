AC_DEFUN([GXX0X],[
  AC_REQUIRE([AC_PROG_CXX])
  AC_ARG_ENABLE(gxx0xcheck,
		AC_HELP_STRING([--disable-gxx0xcheck],
                 [try to enable c++0x feature for g++ [[default=yes]]]),
				 [gxx0xcheck=$enableval],
				 [gxx0xcheck=yes])
  if test "x$GXX" = xyes && test "x$gxx0xcheck" = xyes; then
    AC_MSG_CHECKING([whether g++ accepts -std=c++0x])
    ac_save_CXX="$CXX"
    CXX="$CXX -std=c++0x"
	AC_LANG_PUSH([C++])
    AC_TRY_COMPILE([],[],[
      AC_MSG_RESULT(yes)], [
      CXX="$ac_save_CXX"
      AC_MSG_RESULT(no)])
    AC_LANG_POP
  fi
])

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
