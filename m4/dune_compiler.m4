# $Id$
# check for gcc >= 3.4.1

AC_DEFUN([DUNE_CHECK_COMPILER],[
AC_REQUIRE([AC_PROG_CXX])
cat >conftest.cc <<_ACEOF
#ifdef __GNUC__
#if __GNUC__ > 3 || \
   (__GNUC__ == 3 && (__GNUC_MINOR__ > 4 || \
      (__GNUC_MINOR__ == 4 && \
       __GNUC_PATCHLEVEL__ >= 1)))
#else
#error You need gcc version >= 3.4.1
#endif
#endif
int main() { return 0; }
_ACEOF
if $CXX conftest.cc -o conftest.$ac_exeext >&5; then
  rm -f conftest.$ac_exeext
else
  AC_MSG_ERROR([When using gcc, you need gcc-version >= 3.4.1])    
fi
])