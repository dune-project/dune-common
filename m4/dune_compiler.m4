# $Id$
# check for supported compilers

AC_DEFUN([DUNE_CHECK_COMPILER],[

AC_ARG_ENABLE(compilercheck,
  AC_HELP_STRING([--enable-compilercheck],
                 [check for supported compilers \[default=yes\]]),
  [compilercheck=$enable],
  [compilercheck=yes]
)

SUPPORTED_COMPILER="gcc (>= 3.4.1) or icc (>= 7.0)"

AC_REQUIRE([AC_PROG_CXX])
cat >conftest.cc <<_ACEOF
#ifdef __xlC__
  #if __xlC__ >= 0x0600
    #define CXX_SUPPORTED
  #endif
#endif
#ifdef __ICC
  #if __ICC >= 700
    #define CXX_SUPPORTED
  #endif
#endif
#ifdef __GNUC__
  #if __GNUC__ > 3 || \
     (__GNUC__ == 3 && (__GNUC_MINOR__ > 4 || \
        (__GNUC_MINOR__ == 4 && \
         __GNUC_PATCHLEVEL__ >= 1)))
    #define CXX_SUPPORTED
  #endif
#endif
#ifndef CXX_SUPPORTED
  #error Your compiler is not officially supprted by dune
#endif
int main() { return 0; }
_ACEOF

if test "x$compilercheck" != "xyes" ; then
  AC_MSG_WARN([compilercheck is disabled. DANGEROUS!])    
else
  if $CXX conftest.cc -o conftest.$ac_exeext >&5; then
    rm -f conftest.$ac_exeext
  else
    AC_MSG_ERROR([Your compiler is not officially supported by dune
                  dune is known to work with $SUPPORTED_COMPILER])
  fi
fi

])
