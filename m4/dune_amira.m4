# $Id$
# searches for amira-headers and libs

AC_DEFUN([DUNE_PATH_AMIRA],[
  AC_REQUIRE([AC_PROG_CXX])

  AC_ARG_WITH(amira,
    AC_HELP_STRING([--with-amira=PATH],[directory with Amira inside]))


# store values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"

## do nothing if --without-amira is used
if test x$with_amira != xno ; then

if test x$with_amira != x ; then
    if test -d $with_amira; then
      # expand tilde / other stuff
      AMIRAROOT=`cd $with_amira && pwd`
    else
      AC_MSG_ERROR([directory $with_amira does not exist])
    fi      
else
  # use some default value...
  AMIRAROOT="/usr/local/amira"
fi

AMIRA_LIB_PATH="$AMIRAROOT/lib"
AMIRA_INCLUDE_PATH="$AMIRAROOT/include"

LDFLAGS="$LDFLAGS -L$AMIRA_LIB_PATH"
CPPFLAGS="$CPPFLAGS -I$AMIRA_INCLUDE_PATH"

AC_LANG_PUSH([C++])

# check for header
AC_CHECK_HEADER([amiramesh/AmiraMesh.h], 
   [AMIRA_CPPFLAGS="-I$AMIRA_INCLUDE_PATH"
	HAVE_AMIRA="1"],
  AC_MSG_WARN([AmiraMesh.h not found in $AMIRA_INCLUDE_PATH/amiramesh]))

CPPFLAGS="$AMIRA_CPPFLAGS"

# if header is found...
if test x$HAVE_AMIRA = x1 ; then
#  AC_CHECK_LIB(amiramesh,[AmiraMesh::read],

   LIBS="$LIBS -lamiramesh"

   AC_LINK_IFELSE(AC_LANG_PROGRAM([#include "amiramesh/AmiraMesh.h"], [AmiraMesh* am = AmiraMesh::read("test");]),
	[AMIRA_LIBS="-lamiramesh"
         AMIRA_LDFLAGS="-L$AMIRA_LIB_PATH"
         LIBS="$LIBS $AMIRA_LIBS"],
	[HAVE_AMIRA="0"
	AC_MSG_WARN(libamiramesh not found!)])
fi

AC_LANG_POP([C++])

## end of amira check (--without wasn't set)
fi

# survived all tests?
if test x$HAVE_AMIRA = x1 ; then
  AC_SUBST(AMIRA_LIBS, $AMIRA_LIBS)
  AC_SUBST(AMIRA_LDFLAGS, $AMIRA_LDFLAGS)
  AC_SUBST(AMIRA_CPPFLAGS, $AMIRA_CPPFLAGS)
  AC_DEFINE(HAVE_AMIRA, 1, [Define to 1 if amiramesh-library is found])

  # add to global list
  DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $AMIRA_LDFLAGS"
  DUNE_PKG_LIBS="$DUNE_PKG_LIBS $AMIRA_LIBS"
  DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $AMIRA_CPPFLAGS"

  # set variable for summary
  with_amira="yes"
else
  AC_SUBST(AMIRA_LIBS, "")
  AC_SUBST(AMIRA_LDFLAGS, "")
  AC_SUBST(AMIRA_CPPFLAGS, "")

  # set variable for summary
  with_amira="no"
fi
  
# also tell automake
AM_CONDITIONAL(AMIRA, test x$HAVE_AMIRA = x1)

# reset old values
LIBS="$ac_save_LIBS"
CPPFLAGS="$ac_save_CPPFLAGS"
LDFLAGS="$ac_save_LDFLAGS"

])
