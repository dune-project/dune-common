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
AC_DEFUN([DUNE_PATH_CUDA],[
  AC_REQUIRE([AC_PROG_CXX])

  AC_ARG_WITH([cuda],
    [AS_HELP_STRING([--with-cuda=PATH],
                   [prefix of CUDA installation])])

  HAVE_CUDA=no
  AS_IF([test x"$with_cuda" = x"no"], [
    AC_MSG_NOTICE([skipping check for CUDA])
  ], [
    AC_CACHE_CHECK([for CUDA], dune_cv_cuda_path, [
      dune_cv_cuda_path=no
      AS_IF([test x"$with_cuda" = x || test x"$with_cuda" = xyes], [
        for d in /usr /usr/local; do
          AC_MSG_NOTICE([searching for CUDA in $d...])
          DUNE_CHECK_PATH_CUDA($d)
          AS_IF([test $HAVE_CUDA = yes],[
            dune_cv_cuda_path=$d
            break
          ])
        done],[
        DUNE_CHECK_PATH_CUDA($with_cuda)
        AS_IF([test $HAVE_CUDA = yes],[
          dune_cv_cuda_path=$with_cuda
        ])
      ])
    ])

    if test "x$dune_cv_cuda_path" != xno; then
      HAVE_CUDA=yes
      CUDA_CPPFLAGS="-I$dune_cv_gmp_path/include -DENABLE_GMP=1"
    fi
  ])

  AS_IF([test $HAVE_CUDA = yes],[
    AC_DEFINE([HAVE_CUDA],[ENABLE_CUDA],[Was CUDA found?])
  ], [
  ])

  AM_CONDITIONAL(CUDA,[test $HAVE_CUDA = yes])
  DUNE_ADD_SUMMARY_ENTRY([CUDA],[$HAVE_CUDA])
])

AC_DEFUN([DUNE_CHECK_PATH_CUDA],[
  AC_LANG_PUSH([C++])
  ac_save_cxx="$CXX"
  CXX=nvcc

  AC_CHECK_HEADER([cuda.h], [HAVE_CUDA=yes],
    [AC_MSG_WARN([cuda.h not found in $1])])

  AS_IF([test $HAVE_CUDA = yes],[
    DUNE_CHECK_LIB_EXT([$1/lib], gmp,[__gmpz_abs],[],[
      HAVE_CUDA=no
      AC_MSG_WARN(libcuda not found)])
  ])

  AS_IF([test $HAVE_CUDA = yes],[
    DUNE_CHECK_LIB_EXT([$1/lib], gmpxx,[__gmpz_abs],[],[
      HAVE_CUDA=no
      AC_MSG_WARN(libcuda not found)])
  ])

  # CPPFLAGS="$ac_save_CPPFLAGS"
  # LIBS="$ac_save_LIBS"
  CXX="$ac_save_cxx"
  AC_LANG_POP([C++])
])
