# $Id$
# checks for dune-headers and everything they need

dnl AM_PATH_DUNE([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])

AC_DEFUN(AM_PATH_DUNE,
[
  AC_REQUIRE([AC_PROG_CXX])

  # ch
  AC_ARG_WITH(dune,
    AC_HELP_STRING([--with-dune=PATH],[directory with Dune inside]),
dnl expand tilde / other stuff
    eval with_dune=$with_dune)

dnl try with pkg-config
  if test "x$with_dune" != x ; then
dnl retry with dune-dir
    export PKG_CONFIG_PATH=$with_dune
  fi
  PKG_CHECK_MODULES(DUNE, dune,
	ifelse([$2], , :, [$2]), ifelse([$3], , :, [$3]))

dnl call ACTION-IF-NOT-FOUND
dnl ifelse([$3], , :, [$3])
  
])