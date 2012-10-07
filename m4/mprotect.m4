dnl searches for the sys/mman.h header
dnl and checks the mprotect is available

AC_DEFUN([DUNE_SYS_MPROTECT],[
  AC_REQUIRE([AC_PROG_CC])
  AC_LANG_PUSH([C])
  AC_CHECK_HEADER(sys/mman.h,
    AC_DEFINE(HAVE_SYS_MMAN_H, 1,
                        [Define to 1 if you have <sys/mman.h>.])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <sys/mman.h>],[mprotect(0,0,PROT_NONE);])],
      [AC_DEFINE(HAVE_MPROTECT, 1,
                        [Define to 1 if you have the symbol mprotect.])]))
  AC_LANG_POP()
])
