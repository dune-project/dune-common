# $Id$
# checks for dune-headers and everything they need

# TODO
#
# use pkg-config later? Maybe not really worth it, because only ojne -I is
# needed...

#   #export PKG_CONFIG_LIBDIR=$with_dune/dune
#  #PKG_CHECK_MODULES(DUNE, dune)  

AC_DEFUN(DUNE_PATH_DUNE,
[
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([AC_PROG_CXXCPP])

  # switch tests to c++
  AC_LANG_PUSH([C++])

  # the usual option...
  AC_ARG_WITH(dune,
    AC_HELP_STRING([--with-dune=PATH],[directory with Dune inside]),
# expand tilde / other stuff
    eval with_dune=$with_dune)

# is a directory set?
  if test "x$with_dune" != x ; then
    DUNEROOT=$with_dune
  else
    # set default path
    DUNEROOT=/usr/local/include/
  fi

  ac_save_CPPFLAGS="$CPPFLAGS"

  CPPFLAGS="$CPPFLAGS -I$DUNEROOT"

  # test for an arbitrary header
  AC_CHECK_HEADER([dune/common/misc.hh],
    [DUNE_CPPFLAGS="-I$DUNEROOT"
     HAVE_DUNE=1],
    [HAVE_DUNE=0]
  )

  # did we succeed?
  if test x$HAVE_DUNE = x1 ; then
    AC_SUBST(DUNE_CPPFLAGS, $DUNE_CPPFLAGS)
    AC_DEFINE(HAVE_DUNE, 1, [Define to 1 if dune-headers were found])

    # add to global list
    DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $DUNE_CPPFLAGS"
  fi

  # reset previous flags
  CPPFLAGS="$ac_save_CPPFLAGS"

  # restore previous language settings (leave C++)
  AC_LANG_POP([C++])
])