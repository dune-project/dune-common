## -*- autoconf -*-

# DUNE_PATH_GMP()
#
# shell variables
#   with_gmp
#     no or path
#   HAVE_GMP
#     no or yes
#   GMP_CPPFLAGS
#   GMP_LIBS
#
# substitutions
#   GMP_CPPFLAGS
#   GMP_LIBS
#
# defines:
#   HAVE_GMP
#
# conditionals:
#   GMP
AC_DEFUN([DUNE_PATH_GMP],[
  AC_REQUIRE([AC_PROG_CXX])

  AC_ARG_WITH(gmp,
    AS_HELP_STRING([--with-gmp=PATH],
                   [directory to GMP (GNU MP Bignum Library)]))
  AS_IF([test x$with_gmp = x],[with_gmp=/usr])

  HAVE_GMP=no
  GMP_CPPFLAGS="-I$with_gmp/include -DENABLE_GMP=1"
  GMP_LIBS="-L$with_gmp/lib -lgmpxx -lgmp"

  AC_LANG_PUSH([C++])
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_LIBS="$LIBS"

  CPPFLAGS="$CPPFLAGS $GMP_CPPFLAGS"

  AC_CHECK_HEADER([gmpxx.h], [HAVE_GMP=yes],
    AC_MSG_WARN([gmpxx.h not found in $with_gmp]))

  AS_IF([test $HAVE_GMP = yes],[
    DUNE_CHECK_LIB_EXT([$with_gmp/lib], gmp,[__gmpz_abs],[],[
      HAVE_GMP=no
      AC_MSG_WARN(libgmp not found)])
  ])

  CPPFLAGS="$ac_save_CPPFLAGS"
  LIBS="$ac_save_LIBS"
  AC_LANG_POP([C++])

  AS_IF([test $HAVE_GMP = yes],[
    AC_DEFINE([HAVE_GMP],[ENABLE_GMP],[Was GMP found and GMP_CPPFLAGS used?])
    DUNE_ADD_ALL_PKG([GMP], [\${GMP_CPPFLAGS}],
                     [], [\${GMP_LIBS}])
  ], [
    GMP_CPPFLAGS=
    GMP_LIBS=
  ])

  AC_SUBST([GMP_CPPFLAGS])
  AC_SUBST([GMP_LIBS])

  AM_CONDITIONAL(GMP,[test $HAVE_GMP = yes])
  DUNE_ADD_SUMMARY_ENTRY([GMP],[$HAVE_GMP])
])
