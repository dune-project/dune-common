# $Id$
# searches for albert-headers and libs

# grape.h und libgr.a/libgr.so, normalerweise in einem Verzeichnis

#<@strcmp> In meinem simpel-makefile ist es I=/home/grape/lib/  CFLAGS= -I$I 
#          -D_BSD_SOURCE  $(CC) -o grape $O -L$I -L/usr/X11R6/lib -lgr -lGL 
#          -lX11 -lXext -lXi -ldl -lm -rdynamic -Wl,-rpath,$I  
#<@strcmp> Ob man das -D_BSD_SOURCE noch braucht, weiss ich nicht, es gibt 
#          libc-Versionen, wo ein schlichtes #include <rpc/xdr.h> ohne nicht 
#          funktioniert. Was ich fuer einen Fehler halte.
#<@strcmp> Ah, das -Wl,-rpath,$(INSTALL) braucht man, wenn die
#          libgr.so nicht in einem der Standardpfade liegt. In /usr/local/
#          brauchst Dus also nicht.

AC_DEFUN([DUNE_PATH_GRAPE],[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PATH_XTRA])

  AC_REQUIRE([DUNE_PATH_OPENGL])

  AC_ARG_WITH(grape,
    AC_HELP_STRING([--with-grape=PATH],[directory with Grape inside]))

if test "x$X_LIBS" != x ; then
  # store old values
  ac_save_LDFLAGS="$LDFLAGS"
  ac_save_CFLAGS="$CFLAGS"
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_LIBS="$LIBS"
  LIBS="$X_PRE_LIBS $X_LIBS $X_EXTRA_LIBS"

  # is --with-grape=bla used?
  if test x$with_grape != x ; then
    if test -d $with_grape; then
      # expand tilde / other stuff
      GRAPEROOT=`cd $with_grape && pwd`
    else
      AC_MSG_ERROR([directory $with_grape does not exist])
    fi      
  else
    # set some kind of default grape-path...
    GRAPEROOT="/usr/local/grape/"
  fi

  CPPFLAGS="$CPPFLAGS -I$GRAPEROOT"
  LDFLAGS="$LDFLAGS -L$GRAPEROOT"

  # append OpenGL-options if needed
  if test x$have_gl != xno ; then
    LIBS="$LIBS $GL_LIBS"
  fi

  # check for header
  # we have to use CC for checking the header!!
  AC_LANG_PUSH([C])
  AC_CHECK_HEADER([grape.h],
    [GRAPE_CPPFLAGS="-I$GRAPEROOT"
     HAVE_GRAPE="1"],
    AC_MSG_WARN([grape.h not found in $GRAPEROOT!]))
  AC_LANG_POP

  # check for lib if header was found
  if test x$HAVE_GRAPE = x1 ; then
  AC_CHECK_LIB(gr, grape, 
    [GRAPE_LDFLAGS="-L$GRAPEROOT"
     GRAPE_LIBS="-lgr"], 
    [HAVE_GRAPE="0"
     AC_MSG_WARN([libgr not found in $GRAPEROOT!])])
  fi

  # pre-set variable for summary
  with_grape="no"

  # did it work?
  if test x$HAVE_GRAPE = x1 ; then
    AC_SUBST(GRAPE_LIBS, "$LIBS $GRAPE_LIBS")
    AC_SUBST(GRAPE_LDFLAGS, $GRAPE_LDFLAGS)
    AC_SUBST(GRAPE_CPPFLAGS, $GRAPE_CPPFLAGS)
    AC_DEFINE(HAVE_GRAPE, 1, [Define to 1 if grape-library is found])

    # add to global list
    DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $GRAPE_LDFLAGS"
    DUNE_PKG_LIBS="$DUNE_PKG_LIBS $GRAPE_LIBS"
    DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $GRAPE_CPPFLAGS"

    # re-set variable correctly
    with_grape="yes"
  fi

  # also tell automake
  AM_CONDITIONAL(GRAPE, test x$HAVE_GRAPE != x)

  # reset old values
  LIBS="$ac_save_LIBS"
  CFLAGS="$ac_save_CFLAGS"
  CPPFLAGS="$ac_save_CPPFLAGS"
  LDFLAGS="$ac_save_LDFLAGS"
else
  AC_MSG_WARN("no X-libs/headers found, won't check for GRAPE...")
fi
  
])
