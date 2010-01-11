dnl -*- autoconf -*-
# Determine how to make the C preprocessor include a certain file before the
# main input file from the command line.  Sets the following configure
# substitution:
#
# CPPINCLUDEOPT The include option.  This is something like "-include " (note
#   trailing space).  If no such option could determined, this variable will
#   be set to something like "-D DUMMY=".  This means it must be used like
#   CPPFLAGS="$CPPINCLUDEOPT$header" (note: no whitespace between
#   $CPPINCLUDEOPT and $header).
#
# Sets the following automake conditional:
#
# HAVE_CPPINCLUDEOPT
AC_DEFUN([DUNE_CPPINCLUDEOPT],
[
  AC_LANG_PUSH([C])
  AC_REQUIRE([AC_PROG_CPP])

  AC_MSG_CHECKING([for preprocessor option to preinclude a header])

  cat >conftestpreinc.h <<EOF
#define PREINC_WORKS
EOF
  ac_save_CPPFLAGS="$CPPFLAGS"
  HAVE_CPPINCLUDEOPT=no

  if ! test yes = "$HAVE_CPPINCLUDEOPT"; then
    # check for -include
    CPPINCLUDEOPT="-include "
    CPPFLAGS="$ac_save_CPPFLAGS ${CPPINCLUDEOPT}conftestpreinc.h"
    AC_COMPILE_IFELSE([
#ifdef PREINC_WORKS
/* OK */
#else
#error preinc does not work
#endif
    ],[HAVE_CPPINCLUDEOPT=yes])
  fi

  rm -f conftestpreinc.h
  if ! test yes = "$HAVE_CPPINCLUDEOPT"; then
    CPPINCLUDEOPT="-D DUMMY="
  fi
  AC_MSG_RESULT([$HAVE_CPPINCLUDEOPT (${CPPINCLUDEOPT}file.h)])
  AC_SUBST([CPPINCLUDEOPT])
  AM_CONDITIONAL([HAVE_CPPINCLUDEOPT], [test yes = "$HAVE_CPPINCLUDEOPT"])
  CPPFLAGS="$ac_save_CPPFLAGS"
  AC_LANG_POP([C])
])
