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
    AC_HELP_STRING([--with-dune=PATH],[directory with Dune inside]))

  AC_ARG_ENABLE(localdune,
    AC_HELP_STRING([--enable-localdune],[use Dune-headers/lib]))

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

#   # did we find the headers?
#   if test x$HAVE_DUNE = x1 ; then
#     # check for library
#     #
#     # for the devel-mode, we're searching for a .la-file in the
#     # with-dune-directory, otherwise we'll really test for a lib
#     # (installed in a proper directory!)      

#     if test x"$enable_localdune" != xyes ; then
#       ## normal test

#       # !!! should be pkg-config later (which would save the special
#       # header-check above)
#       # !!! insert symbol defined in the libdune
#       AC_CHECK_LIB(dune, ,[HAVE_DUNE=1],[HAVE_DUNE=0])

#     else
#       ## special test for a local installation
#       ac_save_LDFLAGS="$LDFLAGS"
      	
#       if test x$DUNEROOT != x ; then
#         # have a look into the dune-dir
# 	LDFLAGS="$LDFLAGS -L$DUNEROOT/dune/lib"

# 	# only check for a .la-file
# 	if test -s $DUNEROOT/dune/lib/libdune.la ; then
# 	    DUNE_LDFLAGS="-L$DUNEROOT/dune/lib"
# 	    echo found libdune.la, setting LDFLAGS to $DUNE_LDFLAGS

# 	    # provide arguments like normal lib-check
# 	    LIBS="-ldune"
# 	    HAVE_DUNE=1
# 	else
# 	    AC_MSG_ERROR([localdune is enabled but libdune.la was not found. Please compile the library or set a correct --with-dune])
# 	fi
#       else
# 	  AC_MSG_ERROR([--enable-localdune needs a --with-dune-parameter!])
#       fi

#       # reset variable
#       LDFLAGS="$ac_save_LDFLAGS"
#     fi
#   fi

  # did we succeed?
  if test x$HAVE_DUNE = x1 ; then
    AC_SUBST(DUNE_CPPFLAGS, $DUNE_CPPFLAGS)
    AC_SUBST(DUNE_LDFLAGS, $DUNE_LDFLAGS)
    AC_SUBST(DUNE_LIBS, $LIBS)
    AC_DEFINE(HAVE_DUNE, 1, [Define to 1 if dune was found])

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
