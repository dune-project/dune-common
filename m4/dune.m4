# $Id$
# checks for dune-headers and everything they need

# TODO
#
# use pkg-config later? Maybe not really worth it, because only one -I is
# needed right now...

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
    AC_HELP_STRING([--with-dune=PATH],[directory with Dune inside]))

  # backup of flags
  ac_save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS=""

  # is a directory set?
  if test "x$with_dune" != x ; then
    # expand tilde
    eval with_dune="$with_dune"
    if test -d $with_dune ; then
      # expand tilde / other stuff
      DUNEROOT=`cd $with_dune && pwd`
    
      # expand search path (otherwise empty CPPFLAGS)
      CPPFLAGS="-I$DUNEROOT"
    else
      AC_MSG_ERROR([dune-directory $with_dune does not exist])
    fi
  fi

  # test for an arbitrary header
  AC_CHECK_HEADER([dune/common/misc.hh],
    [HAVE_DUNE=1
     DUNE_CPPFLAGS="$CPPFLAGS"],
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