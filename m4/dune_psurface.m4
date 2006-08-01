# searches for psurface-headers and lib

AC_DEFUN([DUNE_PATH_PSURFACE],[
  AC_REQUIRE([AC_PROG_CXX])

  AC_ARG_WITH(psurface,
    AC_HELP_STRING([--with-psurface=PATH],[directory with the psurface library inside]))

# store values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"

## do nothing if --without-psurface is used
if test x$with_psurface != xno ; then

# is --with-psurface=bla used?
if test "x$with_psurface" != x ; then
    if test -d $with_psurface; then
      # expand tilde / other stuff
      PSURFACEROOT=`cd $with_psurface && pwd`
    else
      AC_MSG_ERROR([directory $with_psurface does not exist])
    fi      
fi
if test "x$PSURFACEROOT" = x; then  
    # use some default value...
    PSURFACEROOT="/usr/local/psurface"
fi

PSURFACE_LIB_PATH="$PSURFACEROOT/lib"
PSURFACE_INCLUDE_PATH="$PSURFACEROOT/include"

LDFLAGS="$LDFLAGS -L$PSURFACE_LIB_PATH"
CPPFLAGS="$CPPFLAGS -I$PSURFACE_INCLUDE_PATH"

AC_LANG_PUSH([C++])

# check for header
AC_CHECK_HEADER([psurface.h], 
   [PSURFACE_CPPFLAGS="-I$PSURFACE_INCLUDE_PATH"
	HAVE_PSURFACE="1"],
   [if test "x$with_psurface" != x ; then
    AC_MSG_WARN([psurface.h not found in $PSURFACE_INCLUDE_PATH])
    fi
   ])

CPPFLAGS="$PSURFACE_CPPFLAGS"

# Psurface uses the AmiraMesh data format to read a write parametrized surfaces to disk.
# Therefore, its installation only makes sense if AmiraMesh support is installed as well.
if test x$HAVE_AMIRAMESH != x1 ; then
  AC_MSG_NOTICE(Check for psurface library aborted because AmiraMesh is unavailable!) 
  HAVE_PSURFACE="0"
fi

# if header is found...
if test x$HAVE_PSURFACE = x1 ; then
   LIBS="$LIBS -lpsurface $AMIRAMESH_LDFLAGS $AMIRAMESH_LIBS"

   AC_LINK_IFELSE(AC_LANG_PROGRAM([#include "psurface.h"], [psurface::LoadMesh("label", "filename");]),
	[PSURFACE_LIBS="-lpsurface"
         PSURFACE_LDFLAGS="-L$PSURFACE_LIB_PATH"
         LIBS="$LIBS $PSURFACE_LIBS"],
	[HAVE_PSURFACE="0"
	AC_MSG_WARN(psurface header found, but libpsurface missing!)])
fi

AC_LANG_POP([C++])

## end of psurface check (--without wasn't set)
fi

with_psurface="no"
# survived all tests?
if test x$HAVE_PSURFACE = x1 ; then
  AC_SUBST(PSURFACE_LIBS, $PSURFACE_LIBS)
  AC_SUBST(PSURFACE_LDFLAGS, $PSURFACE_LDFLAGS)
  AC_SUBST(PSURFACE_CPPFLAGS, $PSURFACE_CPPFLAGS)
  AC_DEFINE(HAVE_PSURFACE, 1, [Define to 1 if psurface-library is found])

  # add to global list
  DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $PSURFACE_LDFLAGS"
  DUNE_PKG_LIBS="$DUNE_PKG_LIBS $PSURFACE_LIBS"
  DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $PSURFACE_CPPFLAGS"

  # set variable for summary
  with_psurface="yes"
else
  AC_SUBST(PSURFACE_LIBS, "")
  AC_SUBST(PSURFACE_LDFLAGS, "")
  AC_SUBST(PSURFACE_CPPFLAGS, "")
fi
  
# also tell automake
AM_CONDITIONAL(PSURFACE, test x$HAVE_PSURFACE = x1)

# reset old values
LIBS="$ac_save_LIBS"
CPPFLAGS="$ac_save_CPPFLAGS"
LDFLAGS="$ac_save_LDFLAGS"

])
