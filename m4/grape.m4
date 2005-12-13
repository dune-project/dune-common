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

# store old values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"

# don't even start testing if X wasn't found
if test "x$X_LIBS" != x && test x$with_grape != xno ; then

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

  # check for header
  # we have to use CC for checking the header!!
  AC_LANG_PUSH([C])
  AC_CHECK_HEADER([grape.h],
    [GRAPE_CPPFLAGS="-I$GRAPEROOT"
     HAVE_GRAPE="1"])
  AC_LANG_POP

  # check for lib if header was found
  if test x$HAVE_GRAPE = x1 ; then
    # if GL was found, add it implicitly...
    #   This is not the best choice, but testing without GL first and
    #   then trying again fails due to caching...
    CPPFLAGS="$GRAPE_CPPFLAGS $GL_CFLAGS"
    LIBS="$LIBS $GL_LIBS -lXext"
    LDFLAGS="$LDFLAGS $GL_LDFLAGS"

    AC_CHECK_LIB(gr, grape, 
      [GRAPE_LDFLAGS="-L$GRAPEROOT $GL_LDFLAGS"
       GRAPE_CPPFLAGS="$CPPFLAGS"
       GRAPE_LIBS="-lgr $GL_LIBS -lXext"], 
      [HAVE_GRAPE="0"])
  fi

  # did it work?
  if test x$HAVE_GRAPE = x1 ; then
    AC_SUBST(GRAPE_LIBS, $GRAPE_LIBS)
    AC_SUBST(GRAPE_LDFLAGS, $GRAPE_LDFLAGS)
    AC_SUBST(GRAPE_CPPFLAGS, $GRAPE_CPPFLAGS)
    AC_DEFINE(HAVE_GRAPE, 1, [Define to 1 if grape-library is found])

    # add to global list
    DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $GRAPE_LDFLAGS"
    DUNE_PKG_LIBS="$DUNE_PKG_LIBS $GRAPE_LIBS"
    DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $GRAPE_CPPFLAGS"
  fi
fi

# report to summary
if test x$HAVE_GRAPE = x1 ; then
  with_grape="yes"
else
  AC_MSG_WARN([X libs,includes not found, skipping Grape test !!!])
  with_grape="no"
fi

# also tell automake	
AM_CONDITIONAL(GRAPE, test x$HAVE_GRAPE = x1)

# reset old values
LIBS="$ac_save_LIBS"
CPPFLAGS="$ac_save_CPPFLAGS"
LDFLAGS="$ac_save_LDFLAGS"
  
])
