# $Id$
# check for supported compilers

AC_DEFUN([DUNE_CHECK_COMPILER],[

AC_ARG_ENABLE(compilercheck,
  AS_HELP_STRING([--enable-compilercheck],
                 [check for supported compilers [[default=yes]]]))

SUPPORTED_COMPILER="gcc (>= 3.4.1) or icc (>= 7.0)"

AC_REQUIRE([AC_PROG_CXX])
cat >conftest.cc <<_ACEOF
#include <cstdio>

#if defined __ICC && ! defined CXX_SUPPORTED
  #if __ICC >= 700
    #define CXX_SUPPORTED "icc %2.2f", 1.0*__ICC/100
  #endif
#endif
#if defined __GNUC__ && ! defined CXX_SUPPORTED
  #if __GNUC__ > 3 || \
     (__GNUC__ == 3 && (__GNUC_MINOR__ > 4 || \
        (__GNUC_MINOR__ == 4 && \
         __GNUC_PATCHLEVEL__ >= 1)))
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

AS_IF([test "x$compilercheck" != "xno"],[
  AC_MSG_WARN([compilercheck is disabled. DANGEROUS!])],[
  AS_IF([$CXX conftest.cc -o conftest.$ac_exeext >&5],[
    COMPILER_NAME=`./conftest.$ac_exeext`;
    rm -f conftest.$ac_exeext],[
    AC_MSG_ERROR([Your compiler is not officially supported by dune
                  dune is known to work with $SUPPORTED_COMPILER])
    ]
  )]
)

AS_IF([test -z "$COMPILER_NAME"],[
	COMPILER_NAME="unknown compiler"])
])
