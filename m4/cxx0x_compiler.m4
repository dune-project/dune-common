# whether g++ accepts -std=c++0x

AC_DEFUN([GXX0X],[
  ac_save_CXX="$CXX"
  AC_CACHE_CHECK([whether g++ accepts -std=c++0x], dune_cv_gplusplus_accepts_cplusplus0x, [
    AC_REQUIRE([AC_PROG_CXX])
    AC_ARG_ENABLE(gxx0xcheck,
      AC_HELP_STRING([--disable-gxx0xcheck],
        [try to enable c++0x feature for g++ [[default=yes]]]),
        [gxx0xcheck=$enableval],
        [gxx0xcheck=yes])
    if test "x$GXX" = xyes && test "x$gxx0xcheck" = xyes; then
      AC_LANG_PUSH([C++])
      CXX="$CXX -std=c++0x"
      AC_TRY_COMPILE([
        #include <iostream>
        #include <array>
        ], [],
        dune_cv_gplusplus_accepts_cplusplus0x=yes,
        dune_cv_gplusplus_accepts_cplusplus0x=no)
      AC_LANG_POP
    fi
  ])
  if test "x$dune_cv_gplusplus_accepts_cplusplus0x" == "xyes" ; then
    CXX="$ac_save_CXX -std=c++0x"
    CXXCPP="$CXXCPP -std=c++0x"
  else
    CXX="$ac_save_CXX"
  fi
])
