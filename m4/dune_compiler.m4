# check for supported compilers

AC_DEFUN([DUNE_CHECK_COMPILER],[

AC_ARG_ENABLE(compilercheck,
  AS_HELP_STRING([--disable-compilercheck],
                 [disable check for supported compilers]),
  [compilercheck=$enableval], [compilercheck=yes])

SUPPORTED_COMPILER="gcc (>= 4.4), should work with recent versions of icc (>= 13) and clang (>= 3.2)"

AC_REQUIRE([AC_PROG_CXX])
cat >conftest.cc <<_ACEOF
#include <cstdio>

#if defined __ICC && ! defined CXX_SUPPORTED
  #if __ICC >= 1300
    #define CXX_SUPPORTED "icc %2.2f", 1.0*__ICC/100
  #endif
#endif
#if defined __clang__ && ! defined CXX_SUPPORTED
  #if __clang_major__ > 3 || (__clang_major__ == 3 && __clang_minor__ >= 2)
    #define CXX_SUPPORTED \
      "clang %i.%i.%i", __clang_major__, __clang_minor__, __clang_patchlevel__
  #endif
#endif
#if defined __GNUC__ && ! defined CXX_SUPPORTED
  #if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4)
    #define CXX_SUPPORTED \
      "gcc %i.%i.%i", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__
  #endif
#endif
#ifndef CXX_SUPPORTED
  #error Your compiler is not officially supported by dune
#endif
int main() { 
  printf(CXX_SUPPORTED);
  return 0;
}
_ACEOF

AS_IF([test "x$compilercheck" = "xno"],
  [AC_MSG_WARN([compilercheck is disabled. DANGEROUS!])],
  [ AC_MSG_CHECKING([whether compiler is officially supported by DUNE])
    AS_IF([$CXX conftest.cc -o conftest.$ac_exeext >&5],
      [ AC_MSG_RESULT([yes])
        COMPILER_NAME=`./conftest.$ac_exeext`;
        rm -f conftest.$ac_exeext],
      [ AC_MSG_RESULT([no])
        AC_MSG_ERROR([Your compiler is not officially supported by dune
                    dune is known to work with $SUPPORTED_COMPILER])
      ])
  ])

AS_IF([test -z "$COMPILER_NAME"],[
  COMPILER_NAME="unknown compiler"])
])
