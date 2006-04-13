#! /bin/sh
# $Id$
# checks for dune-headers and everything they need

# TODO
#
# - use pkg-config if --enable-localdune is not provided

#   #export PKG_CONFIG_LIBDIR=$with_dune/dune
#  #PKG_CHECK_MODULES(DUNE, dune)  

AC_DEFUN([DUNE_CHECK_MODULES],[
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([AC_PROG_CXXCPP])

  # ____DUNE_CHECK_MODULES_____ ($1)

  m4_define(_dune_module,$1)
  m4_define(_DUNE_MODULE,m4_toupper($1))
  m4_define(_dune_header,$2)
  m4_define(_dune_ldpath,$3)
  m4_define(_dune_lib,$3)
  m4_define(_dune_symbol,$4)

  # switch tests to c++
  AC_LANG_PUSH([C++])

  # the usual option...
  AC_ARG_WITH([_dune_module],
    AC_HELP_STRING([--with-_dune_module=PATH],[_dune_module directory]))

  # backup of flags
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_LIBS="$LIBS"
  CPPFLAGS=""
  LIBS=""

  # is a directory set?
  if test $withval != x ; then
    # expand tilde
    if test -d $withval ; then
      # expand tilde / other stuff
      _DUNE_MODULE[]ROOT=`cd $withval && pwd`

      # expand search path (otherwise empty CPPFLAGS)
      CPPFLAGS="-I$_DUNE_MODULE[]ROOT"
    else
      AC_MSG_ERROR([_dune_module-directory $withval does not exist])
    fi
  fi

  # test for an arbitrary header
  AC_CHECK_HEADER([dune/_dune_header],
    [HAVE_[]_DUNE_MODULE=1
     _DUNE_MODULE[]_CPPFLAGS="$CPPFLAGS"],
    [HAVE_[]_DUNE_MODULE=0]
  )

  ## check for lib (if lib name was provided)
  ifelse(_dune_lib,,,[
    # did we find the headers?
    if test x$HAVE[]_DUNE_MODULE = x1 ; then
      ac_save_LDFLAGS="$LDFLAGS"
      ac_save_LIBS="$LIBS"

      ## special test for a local installation
      if test x$_DUNE_MODULE[]ROOT != x ; then
        # have a look into the dune module directory
        LDFLAGS="$LDFLAGS -L$_DUNE_MODULE[]ROOT/dune/_dune_ldpath"

        # only check for a .la-file
        if test -s $_DUNE_MODULE[]ROOT/_dune_ldpath/lib[]_dune_lib[].la ; then
            _DUNE_MODULE[]_LDFLAGS="-L$_DUNE_MODULE[]ROOT/_dune_ldpath"
            echo found lib[]_dune_lib.la, setting LDFLAGS to _DUNE_MODULE[]_LDFLAGS

            # provide arguments like normal lib-check
            _DUNE_MODULE[]_LIBS="-l[]_dune_lib"
            HAVE_[]_DUNE_MODULE=1
        fi
      fi

      ## normal test for a systemwide install
      if test x$HAVE[]_DUNE_MODULE = x0 ; then
         # !!! should be pkg-config later (which would save the special
         # header-check as well)

         # Beware! Untested!!!
         LIBS="-l[]_dune_lib"
         AC_TRY_LINK(,_dune_symbol,
              [HAVE_[]_DUNE_MODULE=1
               _DUNE_MODULE[]_LIBS="$LIBS"],
              [HAVE_[]_DUNE_MODULE=0]
          )
      fi

      # reset variables
      LDFLAGS="$ac_save_LDFLAGS"
      LIBS="$ac_save_LIBS"
    fi
  ])

  # did we succeed?
  if test x$HAVE_[]_DUNE_MODULE = x1 ; then
    AC_SUBST(_DUNE_MODULE[]_CPPFLAGS, $_DUNE_MODULE_[]_CPPFLAGS)
    AC_SUBST(_DUNE_MODULE[]_LDFLAGS, $_DUNE_MODULE[]_LDFLAGS)
    AC_SUBST(_DUNE_MODULE[]_LIBS, $_DUNE_MODULE[]_LIBS)
    AC_DEFINE(HAVE_[]_DUNE_MODULE, 1, [Define to 1 if _dune_module was found])
    AC_SUBST(_DUNE_MODULE[]ROOT, $_DUNE_MODULE[]ROOT)
	
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

AC_DEFUN([DUNE_ALL_MODULES],[
  DUNE_CHECK_MODULES([dunecommon], [common/stdstreams.hh], [common], [common], [Dune::derr.active();])
  DUNE_CHECK_MODULES([duneistl], [istl/allocator.hh])
])

AC_DEFUN([DUNE_DEV_MODE],[
  AC_ARG_ENABLE(dunedevel,
    AC_HELP_STRING([--enable-dunedevel],[activate Dune-Developer-mode]))

  if test x"$enable_dunedevel" = xyes ; then
    AC_DEFINE(DUNE_DEVEL_MODE, 1, [Activates developer output])
  fi
])
