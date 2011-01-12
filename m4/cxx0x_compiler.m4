AC_DEFUN([GXX0X],[
  AC_REQUIRE([AC_PROG_CXX])
  AC_ARG_ENABLE(gxx0xcheck,
		AC_HELP_STRING([--disable-gxx0xcheck],
                 [try to enable c++0x feature for g++ [[default=yes]]]),
				 [gxx0xcheck=$enableval],
				 [gxx0xcheck=yes])
  if test "x$GXX" = xyes && test "x$gxx0xcheck" = xyes; then
    AC_LANG_PUSH([C++])
    AC_MSG_CHECKING([whether g++ accepts -std=c++0x])
    ac_save_CXX="$CXX"
    CXX="$CXX -std=c++0x"
    HAVE_CXX0X=no
    AC_TRY_COMPILE([#include <iostream>
#include <array>],[],[HAVE_CXX0X=yes],[])
    if test "x$HAVE_CXX0X" == "xyes" ; then
      CXXCPP="$CXXCPP -std=c++0x"
    else
      CXX="$ac_save_CXX"
    fi
    AC_MSG_RESULT([$HAVE_CXX0X])
    AC_LANG_POP
  fi
])
