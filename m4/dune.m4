#! /bin/sh
# $Id$
# checks for dune-headers and everything they need

# TODO
#
# - use pkg-config if --enable-localdune is not provided

#   #export PKG_CONFIG_LIBDIR=$with_dune/dune
#  #PKG_CHECK_MODULES(DUNE, dune)  

AC_DEFUN([DUNE_PATH_DUNE],[
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([AC_PROG_CXXCPP])

  # switch tests to c++
  AC_LANG_PUSH([C++])

  # the usual option...
  AC_ARG_WITH(dune,
    AC_HELP_STRING([--with-dune=PATH],[dune/-directory or the one above]))

  AC_ARG_ENABLE(dunedevel,
    AC_HELP_STRING([--enable-dunedevel],[activate Dune-Developer-mode]))

  # backup of flags
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_LIBS="$LIBS"
  CPPFLAGS=""
  LIBS=""

  # is a directory set?
  if test "x$with_dune" != x ; then
    # expand tilde
    eval with_dune="$with_dune"
    if test -d $with_dune ; then
      # expand tilde / other stuff
      DUNEROOT=`cd $with_dune && pwd`

      # automagically use directory above if complete Dune-dir was supplied
      if test `basename $DUNEROOT` = "dune" ; then
        DUNEROOT=`cd $DUNEROOT/.. && pwd`
      fi
    
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

  # did we find the headers?
  if test x$HAVE_DUNE = x1 ; then
      ac_save_LDFLAGS="$LDFLAGS"
      ac_save_LIBS="$LIBS"

      ## special test for a local installation
      if test x$DUNEROOT != x ; then
        # have a look into the dune-dir
	LDFLAGS="$LDFLAGS -L$DUNEROOT/dune/lib"

 	# only check for a .la-file
 	if test -s $DUNEROOT/dune/lib/libdune.la ; then
 	    DUNE_LDFLAGS="-L$DUNEROOT/dune/lib"
 	    echo found libdune.la, setting LDFLAGS to $DUNE_LDFLAGS

 	    # provide arguments like normal lib-check
 	    DUNE_LIBS="-ldune"
 	    HAVE_DUNE=1
	fi
      fi

      ## normal test for a systemwide install
      if test x$HAVE_DUNE = x0 ; then
         # !!! should be pkg-config later (which would save the special
         # header-check as well)

	 # Beware! Untested!!!
	 LIBS="-ldune"
	 AC_TRY_LINK(,[Dune::derr.active();],
              [HAVE_DUNE=1
               DUNE_LIBS="$LIBS"],
              [HAVE_DUNE=0]
          )
      fi

      # reset variables
      LDFLAGS="$ac_save_LDFLAGS"
      LIBS="$ac_save_LIBS"
  fi

  # did we succeed?
  if test x$HAVE_DUNE = x1 ; then
    AC_SUBST(DUNE_CPPFLAGS, $DUNE_CPPFLAGS)
    AC_SUBST(DUNE_LDFLAGS, $DUNE_LDFLAGS)
    AC_SUBST(DUNE_LIBS, $DUNE_LIBS)
    AC_DEFINE(HAVE_DUNE, 1, [Define to 1 if dune was found])

    if test x"$enable_dunedevel" = xyes ; then
      AC_DEFINE(DUNE_DEVEL_MODE, 1, [Activates developer output])
    fi

    # add to global list
    DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $DUNE_CPPFLAGS"
    DUNE_PKG_LIBS="$DUNE_PKG_LIBS $LIBS"
    DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $DUNE_LDFLAGS"
  fi

  # reset previous flags
  CPPFLAGS="$ac_save_CPPFLAGS"
  LIBS="$ac_save_LIBS"

  # restore previous language settings (leave C++)
  AC_LANG_POP([C++])
])
