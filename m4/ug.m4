#! /bin/bash
# $Id$
# searches for UG headers and libs

AC_DEFUN([DUNE_PATH_UG],[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PATH_XTRA])
  AC_REQUIRE([DUNE_DIMENSION])
  AC_REQUIRE([DUNE_MPI])

  AC_ARG_WITH(ug,
    AC_HELP_STRING([--with-ug=PATH],[directory with UG inside]))

  # store old values
  ac_save_LDFLAGS="$LDFLAGS"
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_LIBS="$LIBS"
  
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
      
      # intermediate variables
      UG_LIB_PATH="$UGROOT/lib"
      UG_INCLUDE_PATH="$UGROOT/include"
      
      UG_LDFLAGS="-L$UG_LIB_PATH"

      # set variables so that tests can use them
      LDFLAGS="$LDFLAGS -L$UG_LIB_PATH"
      CPPFLAGS="$CPPFLAGS -I$UG_INCLUDE_PATH"

      # check for central header
      AC_CHECK_FILE([$UG_INCLUDE_PATH/gm.h],
	  [UG_CPPFLAGS="-I$UG_INCLUDE_PATH"
	      HAVE_UG="1"],
	  [HAVE_UG="0"]
      )

#       # check if config is found
#       if test x$HAVE_UG = x1 ; then
# 	  AC_CHECK_FILE([$UGROOT/ug.conf],
# 	      [UG_CONF="$UGROOT/ug.conf"],
# 	      [HAVE_UG="0"])
#       fi

# # define local function for getting the Makefile-Variable-Values from ug.conf
# dune_ug_getopt () {
#     retval=`make -f - <<EOF
# include $UG_CONF
# all:
# 	@echo \\$($UGCONFVAL)
# EOF`
# }

#       if test x$HAVE_UG = x1 ; then
#           echo Reading ug.conf...

#           echo -n "UG-domain-module... "
#           UGCONFVAL="DOM_MODULE"
#           dune_ug_getopt
#           UGDOMAIN=$retval
#           echo $UGDOMAIN      

#           # the libs have a single character defining the domain
#           case "$UGDOMAIN" in
#               STD_DOMAIN)  UGDCHAR=S ;;
#               LGM_DOMAIN)  UGDCHAR=L ;;
#               *)   AC_MSG_ERROR([I don't know this UG-domain-module!]) ;;
#           esac

#           echo -n "UG-GRAPE-bindings... "
#           UGCONFVAL="GRAPE"
#           dune_ug_getopt
#           UGGRAPE=$retval
#           echo $UGGRAPE
#           # !!! check for grape-lib if this is set!

#           if test "$with_problem_dim" != "$with_world_dim" ; then
# 	      AC_MSG_ERROR([problem-dimension and world-dimension have to be the same for UG!])
#           fi
#           UG_DIM="$with_problem_dim"

#           # use full options for link-test
#           UG_CPPFLAGS="-D_${UGDOMAIN}_ -D_${UG_DIM} $UG_CPPFLAGS"
#       fi

      # use global dimension
      if test "$with_problem_dim" != "$with_world_dim" ; then
	  AC_MSG_ERROR([problem-dimension and world-dimension have to be the same for UG!])
      fi
      UG_DIM="$with_problem_dim"
      UG_CPPFLAGS="${UG_CPPFLAGS} -D_${UG_DIM}"

      AC_LANG_PUSH([C++])
      if test x$HAVE_UG = x1 ; then

	  CPPFLAGS="$UG_CPPFLAGS"
	  UG_LIBS="-lug$UG_DIM -ldomS$UG_DIM -lgg$UG_DIM -ldevS"
	  
	  AC_MSG_CHECKING([libug$UG_DIM (without MPI)])
	  LIBS="$UG_LIBS"
          AC_TRY_LINK(
              [#define INT int
               #include "initug.h"],
	      [int i = UG${UG_DIM}d::InitUg(0,0)],
              [UG_LDFLAGS="$LDFLAGS"
	       HAVE_UG="1"
	       AC_MSG_RESULT(yes)
              ],
              [AC_MSG_RESULT(no)
	       HAVE_UG="0"]
	      )

	  # sequential lib not found/does not work?
	  if test x$HAVE_UG != x1 && test x"$MPI_LDFLAGS" != x"" ; then
	    # try again with added MPI-libs
	    UG_LIBS="$UG_LIBS $MPI_LDFLAGS"
	    AC_MSG_CHECKING([libug$UG_DIM (with MPI)])
	    LIBS="$UG_LIBS"
            AC_TRY_LINK(
              [#define INT int
               #include "initug.h"],
	      [int i = UG${UG_DIM}d::InitUg(0,0)],
              [UG_LDFLAGS="$LDFLAGS"
	       UG_CPPFLAGS="$UG_CPPFLAGS -DModelP"
	       HAVE_UG="1"
	       AC_MSG_RESULT(yes)
              ],
              [AC_MSG_RESULT(no)
	       HAVE_UG="0"]
	      )
	  fi

      fi
      AC_LANG_POP([C++])
      
      # pre-set variable for summary
      with_ug="no"
      
      # did it work?
      if test x$HAVE_UG = x1 ; then
	  AC_SUBST(UG_LDFLAGS, $UG_LDFLAGS)
	  AC_SUBST(UG_LIBS, $UG_LIBS)
	  AC_SUBST(UG_CPPFLAGS, $UG_CPPFLAGS)
	  AC_DEFINE(HAVE_UG, 1, [Define to 1 if UG is found])
	  
    # add to global list
	  DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $UG_LDFLAGS"
	  DUNE_PKG_LIBS="$DUNE_PKG_LIBS $UG_LIBS"
	  DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $UG_CPPFLAGS"
	  
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

dnl Local Variables:
dnl mode: shell-script
dnl End:
