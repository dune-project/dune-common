# whether compiler accepts -std=c++11 or -std=c++0x
# can be disabled by --disable-cxx11check

AC_DEFUN([CXX11],[
  AC_REQUIRE([AC_PROG_CXX])
  AC_ARG_ENABLE(cxx11check,
    AC_HELP_STRING([--disable-cxx11check],
      [Skip test whether compiler supports flag -std=c++11 and -std=c++0x to enable C++11 features
       (Dune always requires C++11 support - if your compiler requires custom flags for C++11 mode, tell configure about them
       in your CXXFLAGS and disable this check)]),
      [cxx11check=$enableval],
      [cxx11check=yes])

  # try flag -std=c++11
  AC_CACHE_CHECK([whether $CXX accepts -std=c++11], dune_cv_gplusplus_accepts_cplusplus11, [
    dune_cv_gplusplus_accepts_cplusplus11=no
    if test "x$GXX" = xyes && test "x$cxx11check" = xyes; then
      ac_save_CXX="$CXX"
      AC_LANG_PUSH([C++])
      CXX="$CXX -std=c++11"
      AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
        #include <iostream>
        #include <array>
        ]], [])],
        dune_cv_gplusplus_accepts_cplusplus11=yes,
        dune_cv_gplusplus_accepts_cplusplus11=no)
      AC_LANG_POP([C++])
      CXX="$ac_save_CXX"
    fi
  ])
  # try flag -std=c++0x
  if test "x$dune_cv_gplusplus_accepts_cplusplus11" == "xno" ; then
    AC_CACHE_CHECK([whether $CXX accepts -std=c++0x], dune_cv_gplusplus_accepts_cplusplus0x, [
      dune_cv_gplusplus_accepts_cplusplus0x=no
      if test "x$GXX" = xyes && test "x$cxx11check" = xyes; then
        ac_save_CXX="$CXX"
        AC_LANG_PUSH([C++])
        CXX="$CXX -std=c++0x"
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
          #include <iostream>
          #include <array>
          ]], [])],
          dune_cv_gplusplus_accepts_cplusplus0x=yes,
          dune_cv_gplusplus_accepts_cplusplus0x=no)
        AC_LANG_POP([C++])
        CXX="$ac_save_CXX"
      fi
    ])
  fi

  if test "x$dune_cv_gplusplus_accepts_cplusplus11" == "xyes" ; then
    CXX="$CXX -std=c++11"
    CXXCPP="$CXXCPP -std=c++11"
  elif test "x$dune_cv_gplusplus_accepts_cplusplus0x" == "xyes" ; then
    CXX="$CXX -std=c++0x"
    CXXCPP="$CXXCPP -std=c++0x"
  else
    if test "x$cxx11check" = xyes; then
      # we ran the test, and it failed - bail out
      AC_MSG_FAILURE([Your compiler does not seem to support C++11!])
    else
      # test was skipped, warn the user
      AC_MSG_WARN([Skipping test for C++11 support, make sure your compiler supports at least the feature set of GCC 4.4!])
    fi
  fi

  # set feature support macros for backwards compatibility
  AC_DEFINE(HAVE_VARIADIC_TEMPLATES, 1, [THIS MACRO IS DEPRECATED AND ONLY KEPT FOR BACKWARDS COMPATIBILITY UNTIL THE NEXT RELEASE!])
  AC_DEFINE(HAVE_VARIADIC_CONSTRUCTOR_SFINAE, 1, [THIS MACRO IS DEPRECATED AND ONLY KEPT FOR BACKWARDS COMPATIBILITY UNTIL THE NEXT RELEASE!])
  AC_DEFINE(HAVE_RVALUE_REFERENCES, 1, [THIS MACRO IS DEPRECATED AND ONLY KEPT FOR BACKWARDS COMPATIBILITY UNTIL THE NEXT RELEASE!])

])
