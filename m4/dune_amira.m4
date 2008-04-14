# $Id$
# searches for amiramesh-headers and libs

AC_DEFUN([DUNE_PATH_AMIRAMESH],[
  AC_REQUIRE([AC_PROG_CXX])

  AC_ARG_WITH(amiramesh,
    AC_HELP_STRING([--with-amiramesh=PATH],[directory with AmiraMesh inside]))

# store values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"

## do nothing if --without-amiramesh is used
if test x$with_amiramesh != xno ; then

# is --with-amiramesh=bla used?
if test "x$with_amiramesh" != x ; then
	if ! test -d $with_amiramesh; then
        AC_MSG_WARN([Amiramesh directory $with_amiramesh does not exist])
	else
        # expand tilde / other stuff
		AMIRAMESHROOT=`cd $with_amiramesh && pwd`
	fi
fi
if test "x$AMIRAMESHROOT" = x; then
    # use some default value...
    AMIRAMESHROOT="/usr/local/amiramesh"
fi

AMIRAMESH_LIB_PATH="$AMIRAMESHROOT/lib"
AMIRAMESH_INCLUDE_PATH="$AMIRAMESHROOT/include"

LDFLAGS="$LDFLAGS -L$AMIRAMESH_LIB_PATH"
CPPFLAGS="$CPPFLAGS -I$AMIRAMESH_INCLUDE_PATH"

AC_LANG_PUSH([C++])

# check for header
AC_CHECK_HEADER([amiramesh/AmiraMesh.h], 
   [AMIRAMESH_CPPFLAGS="-I$AMIRAMESH_INCLUDE_PATH"
	HAVE_AMIRAMESH="1"],
  AC_MSG_WARN([AmiraMesh.h not found in $AMIRAMESH_INCLUDE_PATH/amiramesh]))

CPPFLAGS="$AMIRAMESH_CPPFLAGS"

# if header is found...
if test x$HAVE_AMIRAMESH = x1 ; then
   LIBS="$LIBS -lamiramesh"

   AC_LINK_IFELSE(AC_LANG_PROGRAM([#include "amiramesh/AmiraMesh.h"], [AmiraMesh* am = AmiraMesh::read("test");]),
	[AMIRAMESH_LIBS="-lamiramesh"
         AMIRAMESH_LDFLAGS="-L$AMIRAMESH_LIB_PATH"
         LIBS="$LIBS $AMIRAMESH_LIBS"],
	[HAVE_AMIRAMESH="0"
	AC_MSG_WARN(libamiramesh not found!)])
fi

AC_LANG_POP([C++])

## end of amiramesh check (--without wasn't set)
fi

with_amiramesh="no"
# survived all tests?
if test x$HAVE_AMIRAMESH = x1 ; then
  AC_SUBST(AMIRAMESH_LIBS, $AMIRAMESH_LIBS)
  AC_SUBST(AMIRAMESH_LDFLAGS, $AMIRAMESH_LDFLAGS)
  AC_SUBST(AMIRAMESH_CPPFLAGS, $AMIRAMESH_CPPFLAGS)
  AC_DEFINE(HAVE_AMIRAMESH, 1, [Define to 1 if amiramesh-library is found])

  # add to global list
  DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $AMIRAMESH_LDFLAGS"
  DUNE_PKG_LIBS="$DUNE_PKG_LIBS $AMIRAMESH_LIBS"
  DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $AMIRAMESH_CPPFLAGS"

  # set variable for summary
  with_amiramesh="yes"
else
  AC_SUBST(AMIRAMESH_LIBS, "")
  AC_SUBST(AMIRAMESH_LDFLAGS, "")
  AC_SUBST(AMIRAMESH_CPPFLAGS, "")
fi
  
# also tell automake
AM_CONDITIONAL(AMIRAMESH, test x$HAVE_AMIRAMESH = x1)

# reset old values
LIBS="$ac_save_LIBS"
CPPFLAGS="$ac_save_CPPFLAGS"
LDFLAGS="$ac_save_LDFLAGS"

DUNE_ADD_SUMMARY_ENTRY([AmiraMesh],[$with_amiramesh])

])
