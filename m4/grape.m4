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

AC_DEFUN(DUNE_PATH_GRAPE,
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PATH_XTRA])

  AC_REQUIRE([DUNE_PATH_OPENGL])

  AC_ARG_WITH(grape,
    AC_HELP_STRING([--with-grape=PATH],[directory with Grape inside]),
dnl expand tilde / other stuff
    eval with_grape=$with_grape)
dnl extract absolute path
dnl eval with_albert=`cd $with_albert ; pwd`

if test "x$X_LIBS" != x ; then
  # store old values
  ac_save_LDFLAGS="$LDFLAGS"
  ac_save_LIBS="$LIBS"
  LIBS="$X_PRE_LIBS $X_LIBS $X_EXTRA_LIBS"

  # is --with-grape=bla used?
  if test x$with_grape != x ; then
    GRAPEROOT="$with_grape"
    LDFLAGS="$LDFLAGS -L$with_grape"
  else
    # set some kind of default grape-path...
    GRAPEROOT="/usr/local/grape/"
  fi

  # append OpenGL-options if needed
  if test x$have_gl != xno ; then
    LIBS="$LIBS $GL_LIBS"
  fi

  # check for header
  # !! auf AC_CHECK_HEADER umstellen
  AC_CHECK_FILE($GRAPEROOT/grape.h,
    [GRAPE_INCLUDE="-I$GRAPEROOT"
     HAVE_GRAPE="1"],
    AC_MSG_WARN([grape.h not found in $GRAPEROOT!])
  )

  # check for lib
  AC_CHECK_LIB(gr, grape, 
    [LIBS="$LIBS -L$GRAPEROOT -lgr"
     HAVE_GRAPE="1"], 
     AC_MSG_WARN([libgr not found in $GRAPEROOT!]))

  # did it work?
  if test x$HAVE_GRAPE != x ; then
    AC_SUBST(GRAPE_LIBS, $LIBS)
    AC_SUBST(GRAPE_INCLUDE, $GRAPE_INCLUDE)
    AC_DEFINE(HAVE_GRAPE, 1, [Define to 1 if grape-library is found])
  fi

  # also tell automake
  AM_CONDITIONAL(GRAPE, test x$HAVE_GRAPE != x)

  # reset old values
  LIBS="$ac_save_LIBS"
  LDFLAGS="$ac_save_LDFLAGS"
else
  AC_MSG_WARN("no X-libs/headers found, won't check for GRAPE...")
fi
  
])