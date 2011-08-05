# SYNOPSIS
#
#   DUNE_BOOST_BASE([MINIMUM-VERSION], [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#
# DESCRIPTION
#
#   Test for the Boost C++ libraries of a particular version (or newer)
#
#   If no path to the installed boost library is given the macro searchs
#   under /usr, /usr/local, /opt and /opt/local and evaluates the
#   $BOOST_ROOT environment variable. Further documentation is available at
#   <http://randspringer.de/boost/index.html>.
#
#   This macro calls:
#
#     AX_BOOST_BASE / AC_SUBST(BOOST_CPPFLAGS) / AC_SUBST(BOOST_LDFLAGS)
#
#   And sets:
#
#     HAVE_BOOST
#        ENABLE_BOOST or undefined. Whether boost was found. The correct way to
#        to check this is "#if HAVE_BOOST": This way boost featers will be disabled
#        unless ${BOOST_CPPFLAGS} was given when compiling
#serial 1
AC_DEFUN([DUNE_BOOST_BASE],
[
AX_BOOST_BASE([$1],[
  AC_DEFINE(HAVE_BOOST, [ENABLE_BOOST],[Define to ENABLE_BOOST if the Boost library is available])
  BOOST_CPPFLAGS="$BOOST_CPPFLAGS -DENABLE_BOOST"
  AC_SUBST(BOOST_CPPFLAGS)
  # execute ACTION-IF-FOUND (if present):
  ifelse([$2], , :, [$2])
],[
  # execute ACTION-IF-NOT-FOUND (if present):
  ifelse([$3], , :, [$3])
])])