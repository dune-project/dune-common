# $Id$

#################################
# getpagesize 
# -----------
# On some systems, the page size is available as the macro
# PAGE_SIZE in the header file `sys/param.h'. On others, the page size
# is available via the sysconf function. If none of those work, you
# must generally simply guess a value such as 4096. 
#################################

AC_DEFUN([DUNE_GETPAGESIZE],[
AC_REQUIRE([AC_PROG_CC])
AC_MSG_NOTICE([Checking how to determine PAGESIZE])

#################################
# user defined pagesize
#################################
AC_ARG_WITH(pagesize,
  AC_HELP_STRING([--with-pagesize=PAGESIZE],[pagesize of this system]))

if test x$with_pagesize != x ; then
  DUNE_PAGESIZE=$with_pagesize
  AC_MSG_NOTICE([using user defined value])
fi

#################################
# test int getpagesize(void);
#################################
cat >conftest.c <<_ACEOF
#include <unistd.h>
#include <stdio.h>
int main() { printf("%i", getpagesize()); return 0; }
_ACEOF

if test x$DUNE_PAGESIZE == x ; then
  AC_MSG_CHECKING([for int getpagesize(void)])
  if $CC $CFLAGS conftest.c -o conftest.$ac_exeext >&5; then
	DUNE_PAGESIZE=`./conftest.$ac_exeext`
    rm -f conftest.$ac_exeext
  fi
  AC_MSG_RESULT(yes)
fi

#################################
# test int getpagesize(void);
#################################
cat >conftest.c <<_ACEOF
#include <unistd.h>
#include <stdio.h>
int main() { printf("%i", sysconf(_SC_PAGESIZE)); return 0; }
_ACEOF

if test x$DUNE_PAGESIZE == x ; then
  AC_MSG_CHECKING([for int sysconf(_SC_PAGESIZE)])
  if $CC $CFLAGS conftest.c -o conftest.$ac_exeext >&5; then
	DUNE_PAGESIZE=`./conftest.$ac_exeext`
    rm -f conftest.$ac_exeext
  fi
  AC_MSG_RESULT(yes)
fi

#################################
# test int getpagesize(void);
#################################
cat >conftest.c <<_ACEOF
#include <unistd.h>
#include <stdio.h>
int main() { printf("%i", sysconf(_SC_PAGE_SIZE)); return 0; }
_ACEOF

if test x$DUNE_PAGESIZE == x ; then
  AC_MSG_CHECKING([for int sysconf(_SC_PAGE_SIZE)])
  if $CC $CFLAGS conftest.c -o conftest.$ac_exeext >&5; then
	DUNE_PAGESIZE=`./conftest.$ac_exeext`
    rm -f conftest.$ac_exeext
  fi
  AC_MSG_RESULT(yes)
fi

#################################
# test int getpagesize(void);
#################################
cat >conftest.c <<_ACEOF
#include <sys/param.h>
#include <stdio.h>
int main() { printf("%i", PAGE_SIZE); return 0; }
_ACEOF

if test x$DUNE_PAGESIZE == x ; then
  AC_MSG_CHECKING([for definition of PAGE_SIZE in sys/param.h])
  if $CC $CFLAGS conftest.c -o conftest.$ac_exeext >&5; then
	DUNE_PAGESIZE=`./conftest.$ac_exeext`
    rm -f conftest.$ac_exeext
  fi
  AC_MSG_RESULT(yes)
fi

#################################
# fall back to default
#################################
if test x$DUNE_PAGESIZE == x ; then
  DUNE_PAGESIZE=4096
  AC_MSG_WARN([failed to determine PAGE_SIZE, falling back to default 4096])
fi

#################################
# store pagesize & clean up
#################################
AC_DEFINE_UNQUOTED(DUNE_PAGESIZE, $DUNE_PAGESIZE, [Pagesize of this system])

AC_MSG_NOTICE([setting DUNE_PAGESIZE to $DUNE_PAGESIZE])

rm -f conftest.c

])
