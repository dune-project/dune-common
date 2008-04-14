# $Id$
# searches for alberta-headers and libs

AC_DEFUN([DUNE_PATH_ALBERTA],[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PATH_XTRA])
  AC_REQUIRE([DUNE_PATH_OPENGL])
  AC_REQUIRE([DUNE_ALBERTA_DIMENSION])

  AC_ARG_WITH(alberta,
    AC_HELP_STRING([--with-alberta=PATH],[directory with ALBERTA (ALBERTA
    version 1.2 and higher) inside]))
# do not use alberta debug lib 
with_alberta_debug=0

# store old values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"
# LIBS=""

## do nothing if no --with-alberta was supplied
if test x$with_alberta != x && test x$with_alberta != xno ; then

  if test x$with_alberta = xyes ; then
    AC_MSG_ERROR([You have to provide a directory --with-alberta=PATH])
  fi

  # is --with-alberta=bla used?
  if test "x$with_alberta" != x ; then
  if ! test -d $with_alberta; then
        AC_MSG_WARN([ALBERTA directory $with_alberta does not exist])
  else
        # expand tilde / other stuff
    ALBERTAROOT=`cd $with_alberta && pwd`
  fi
  fi
  if test "x$ALBERTAROOT" = x; then
    # use some default value...
    ALBERTAROOT="/usr/local/alberta"
  fi

ALBERTA_LIB_PATH="$ALBERTAROOT/lib"
# take both include paths 
ALBERTA_INCLUDE_PATH="$ALBERTAROOT/include/alberta"
# if path of version 2.0 not exists take old version 
ALBERTA_VERSION="2.0"
if ! test -d $ALBERTA_INCLUDE_PATH ; then 
  ALBERTA_INCLUDE_PATH="$ALBERTAROOT/include -DALBERTA_VERSION_12"
  ALBERTA_VERSION="1.2"
fi

# set variables so that tests can use them
REM_CPPFLAGS=$CPPFLAGS

LDFLAGS="$LDFLAGS -L$ALBERTA_LIB_PATH"
ALBERTADIM="-DDIM=$with_alberta_dim -DDIM_OF_WORLD=$with_alberta_dim"
CPPFLAGS="$CPPFLAGS $ALBERTADIM -DEL_INDEX=0 -I$ALBERTA_INCLUDE_PATH"

# check for header
AC_CHECK_HEADER([alberta.h], 
   [ALBERTA_CPPFLAGS="-I$ALBERTA_INCLUDE_PATH -DENABLE_ALBERTA"
  HAVE_ALBERTA="1"],
  AC_MSG_WARN([alberta.h not found in $ALBERTA_INCLUDE_PATH]))

CPPFLAGS="$REM_CPPFLAGS -I$ALBERTA_INCLUDE_PATH"
REM_CPPFLAGS=

# TODO: check if static flag exists 
# link_static_flag defines the flag for the linker to link only static
# didnt work, with $link_static_flag, so quick hack here

# if header is found...
if test x$HAVE_ALBERTA = x1 ; then
  AC_CHECK_LIB(alberta_util,[alberta_calloc],
  [ALBERTA_LIBS="-lalberta_util"
         ALBERTA_LDFLAGS="-L$ALBERTA_LIB_PATH"
         LIBS="$LIBS $ALBERTA_LIBS"],
  [HAVE_ALBERTA="0"
  AC_MSG_WARN(-lalberta_util not found!)])
fi

# still everything found?
if test x$HAVE_ALBERTA = x1 ; then
  # construct libname
  # the zero is the sign of the no-debug-lib
  # define varaible lib name depending on problem and world dim, to change
  # afterwards easily 
  variablealbertalibname="alberta_$``(``ALBERTA_DIM``)``d"
  albertalibname="alberta_${with_alberta_dim}d"
  AC_CHECK_LIB($albertalibname,[mesh_traverse],
  [ALBERTA_LIBS="-l$variablealbertalibname $ALBERTA_LIBS $ALBERTA_EXTRA"],
  [NEW_ALBERTA_LIB="0"
  AC_MSG_WARN(-l$albertalibname not found!)])
    
  # if new lib not found check old version   
  if test x$NEW_ALBERTA_LIB = x0 ; then 
    variablealbertalibname="ALBERTA$``(``ALBERTA_DIM``)``$``(``ALBERTA_DIM``)``_0"
    albertalibname="ALBERTA${with_alberta_dim}${with_alberta_dim}_${with_alberta_debug}"
    AC_CHECK_LIB($albertalibname,[mesh_traverse],
    [ALBERTA_LIBS="-l$variablealbertalibname $ALBERTA_LIBS $ALBERTA_EXTRA"],
    [HAVE_ALBERTA="0"
    AC_MSG_WARN(-l$albertalibname not found!)])
  fi 
fi

## end of alberta check (--without wasn't set)
fi

# survived all tests?
if test x$HAVE_ALBERTA = x1 ; then
  AC_SUBST(ALBERTA_LIBS, $ALBERTA_LIBS)
  AC_SUBST(ALBERTA_LDFLAGS, $ALBERTA_LDFLAGS)
  AC_SUBST(ALBERTA_CPPFLAGS, $ALBERTA_CPPFLAGS)
  AC_DEFINE(HAVE_ALBERTA, ENABLE_ALBERTA,
    [This is only true if alberta-library was found by configure 
     _and_ if the application uses the ALBERTA_CPPFLAGS])

  # add to global list
  DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $ALBERTA_LDFLAGS"
  DUNE_PKG_LIBS="$DUNE_PKG_LIBS $ALBERTA_LIBS"
  DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $ALBERTA_CPPFLAGS"

  # set variable for summary
  with_alberta="yes (Version $ALBERTA_VERSION)"
else
  AC_SUBST(ALBERTA_LIBS, "")
  AC_SUBST(ALBERTA_LDFLAGS, "")
  AC_SUBST(ALBERTA_CPPFLAGS, "")

  # set variable for summary
  with_alberta="no"
fi
  
# also tell automake
AM_CONDITIONAL(ALBERTA, test x$HAVE_ALBERTA = x1)

# reset old values
LIBS="$ac_save_LIBS"
CPPFLAGS="$ac_save_CPPFLAGS"
LDFLAGS="$ac_save_LDFLAGS"

DUNE_ADD_SUMMARY_ENTRY([ALBERTA],[$with_alberta])

])

# asks for problem-dimension and world-dimension to pass on to Alberta
AC_DEFUN([DUNE_ALBERTA_DIMENSION],[
  AC_REQUIRE([DUNE_GRID_DIMENSION])

AC_ARG_WITH(alberta_dim,
            AC_HELP_STRING([--with-alberta-dim=2|3],
          [dimension of ALBERTA grid (default=grid-dim if delivered otherwise 2)]),,with_alberta_dim=2)

# default dimension of the world coordinates is 2
AC_ARG_WITH(alberta_world_dim,
            AC_HELP_STRING([--with-alberta-world-dim=2|3],
          [dimension of world enclosing the ALBERTA grid (default=alberta-dim)]),,
            with_alberta_world_dim=$with_alberta_dim)

if test x$with_grid_dim != x0 ; then 
  variablealbertdim="$``(``GRIDDIM``)``"
  AC_SUBST(ALBERTA_DIM, $variablealbertdim ) 
else 
  variablealbertdim=$with_alberta_dim
  AC_SUBST(ALBERTA_DIM, $variablealbertdim ) 
fi 

AC_DEFINE_UNQUOTED(ALBERTA_DIM, $with_alberta_dim,
            [Dimension of ALBERTA grid])

])
