#! /bin/bash
# $Id$
# searches for UG headers and libs

# TODO:
#
# - wenn X gefunden ist -> devX, sonst devS
# - $UGROOT auswerten, wenn es schon gesetzt ist
# - ug.conf auswerten

          # IF = X|S
          # DOM_MODULE -> -D...
          # GRAPE
          # DIM? 

AC_DEFUN([DUNE_PATH_UG],[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PATH_XTRA])
  AC_REQUIRE([DUNE_DIMENSION])

  AC_ARG_WITH(ug,
    AC_HELP_STRING([--with-ug=PATH],[directory with UG inside]))

  # store old values
  ac_save_LDFLAGS="$LDFLAGS"
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_LIBS="$LIBS"

  if test "x$X_LIBS" != x ; then
      LIBS="$X_PRE_LIBS -lX11 $X_LIBS $X_EXTRA_LIBS -lXt -lXaw"
  else
      LIBS=""
  fi

  ## do nothing if --without-ug is used
  if test x$with_ug != xno ; then

      # is --with-ug=bla used?
      if test "x$with_ug" != x ; then
          # expand tilde / other stuff
          UGROOT=`cd $with_ug && pwd`
	  if ! test -d $UGROOT; then
	      AC_MSG_ERROR([directory $with_ug does not exist!])
	  fi      
      else
          # use some default value...
	  UGROOT="/usr/local/ug"
      fi

      UG_LIB_PATH="$UGROOT/lib"
      UG_INCLUDE_PATH="$UGROOT/include"
      
      UG_LDFLAGS="-L$UG_LIB_PATH"

      # set variables so that tests can use them
      LDFLAGS="$LDFLAGS -L$UG_LIB_PATH"
      CPPFLAGS="$CPPFLAGS -I$UG_INCLUDE_PATH"

# besserer Test, klappt aber nicht, weil compiler.h auf der
# Kommandozeile ein define mit dem Compilertyp braucht...
#
#      AC_CHECK_HEADER([ugm.h],
#	  [UG_CPPFLAGS="-I$UG_INCLUDE_PATH"
#	      HAVE_UG="1"],
#	  [HAVE_UG="0"]
#      )
      
      # !!! AC_CHECK_FILE sollte hier gehen
      echo -n checking for gm.h... 
      if test -e $UG_INCLUDE_PATH/gm.h ; then
	  UG_CPPFLAGS="-I$UG_INCLUDE_PATH"
	  HAVE_UG="1"
	  echo yes
      else
	  HAVE_UG="0"
	  echo no
      fi

#      if test x$HAVE_UG = x1 ; then
#	  AC_CHECK_LIB([devX], [UserWrite], 
#	      [UG_LDFLAGS="$UG_LDFLAGS -ldevX"],
#	      [HAVE_UG="0"]
#	  )
#      fi

      if test "$with_problem_dim" != "$with_world_dim" ; then
	  AC_MSG_ERROR([problem-dimension and world-dimension have to be the same for UG!])
      fi
      UG_DIM="$with_problem_dim"

#      if test x$HAVE_UG = x1 ; then
#	  AC_CHECK_LIB([domS$UG_DIM], [InitDom],
#	      [UG_LDFLAGS="$UG_LDFLAGS -ldomS$UG_DIM"],
#	      [HAVE_UG="0"],
#	      [$X_LIBS])
#      fi

#      if test x$HAVE_UG = x1 ; then
#	  AC_CHECK_LIB([gg$UG_DIM], [InitGG]
#	      [UG_LDFLAGS="$UG_LDFLAGS -ldomS$UG_DIM"],
#	      [HAVE_UG="0"],
#	      [$X_LIBS])      
#      fi

      if test x$HAVE_UG = x1 ; then
	  AC_CHECK_LIB([ug$UG_DIM],[InitUg],
	      [UG_LDFLAGS="$UG_LDFLAGS"
	       UG_LIBS="-lgrapeOFF$UG_DIM -ldevS -ldomS$UG_DIM -lgg$UG_DIM -lug$UG_DIM"],
	      [HAVE_UG="0"],
	      [-lgrapeOFF$UG_DIM -ldevS -ldomS$UG_DIM -lgg$UG_DIM])
      fi

      # pre-set variable for summary
      with_ug="no"

      # did it work?
      if test x$HAVE_UG = x1 ; then
	  AC_SUBST(UG_LDFLAGS, $UG_LDFLAGS)
	  AC_SUBST(UG_LIBS, $UG_LIBS)
	  # !!! domain !!!
	  AC_SUBST(UG_CPPFLAGS, "-D_$UG_DIM -D_STD_DOMAIN_ $UG_CPPFLAGS")
	  AC_DEFINE(HAVE_UG, 1, [Define to 1 if UG is found])
	  
          # add to global list
	  DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $UG_LDFLAGS"
	  DUNE_PKG_LIBS="$DUNE_PKG_LIBS $UG_LIBS"
	  DUNE_PKG_CPPFLAGS="$DUNE_PKG_CFLAGS $UG_CPPFLAGS"

          # re-set variable correctly
	  with_ug="yes"
      fi 

  # end of "no --without-ug"
  fi
  
  # tell automake	
  AM_CONDITIONAL(UG, test x$HAVE_UG = x1)

  # restore variables
  LDFLAGS="$ac_save_LDFLAGS"
  CPPFLAGS="$ac_save_CPPFLAGS"
  LIBS="$ac_save_LIBS"
  
])

