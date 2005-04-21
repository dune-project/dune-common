#! /bin/bash
# $Id$
# searches for ParMetis headers and libs

AC_DEFUN([DUNE_PATH_PARMETIS],[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PATH_XTRA])
  AC_REQUIRE([DUNE_MPI])

  AC_ARG_WITH(parmetis,
    AC_HELP_STRING([--with-parmetis=PATH],[directory with ParMETIS inside]))

  # store old values
  ac_save_LDFLAGS="$LDFLAGS"
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_LIBS="$LIBS"
  
  ## do nothing if --without-parmetis is used
  if test x"$MPI_LDFLAGS" != x""  && test x$with_parmetis != x && test x$with_parmetis != xno ; then
      
      if test x$with_parmetis == xyes ; then
	  AC_MSG_ERROR([You have to provide a directory --with-parmetis=PATH])
      fi
      
      if test -d $with_parmetis; then
          # expand tilde / other stuff
          PARMETISROOT=`cd $with_parmetis && pwd`
      else
	  AC_MSG_ERROR([directory $with_parmetis does not exist!])
      fi
      
      # intermediate variables
      PARMETIS_LIB_PATH="$PARMETISROOT/"
      PARMETIS_INCLUDE_PATH="$PARMETISROOT/"
      
      PARMETIS_LDFLAGS="-L$PARMETIS_LIB_PATH $MPI_LDFLAGS"

      # set variables so that tests can use them
      LDFLAGS="$LDFLAGS -L$PARMETIS_LIB_PATH $MPI_LDFLAGS"
      CPPFLAGS="$CPPFLAGS -I$PARMETIS_INCLUDE_PATH $MPI_CPPFLAGS"

      # check for central header
      AC_CHECK_HEADER([parmetis.h],[
	      PARMETIS_CPPFLAGS="-I$PARMETIS_INCLUDE_PATH"
	      HAVE_PARMETIS="1"],[
	      HAVE_PARMETIS="0"
	      AC_MSG_WARN([parmetis.h not found in $PARMETIS_INCLUDE_PATH with $CPPFLAGS])]
      )

      PARMETIS_CPPFLAGS="${MPI_CPPFLAGS} ${PARMETIS_CPPFLAGS}"

#      AC_LANG_PUSH([C++])
      
      # if header is found check for the libs
      
      if test x$HAVE_PARMETIS = x1 ; then
	  AC_CHECK_LIB(metis, [metis_partgraphkway],[
		  PARMETIS_LIBS=" -lmetis"
		  PARMETIS_LDFLAGS="-L$PARMETIS_LIB_PATH"
		  LIBS="$LIBS $PARMETIS_LIBS"],[
		  HAVE_PARMETIS="0"
		  AC_MSG_WARN(libmetis not found!)])
      fi

      if test x$HAVE_PARMETIS = x1 ; then
	  AC_CHECK_LIB(parmetis, [parmetis_v3_partkway],[
		  PARMETIS_LIBS="$MPILIBS -lparmetis -lmetis"
		  PARMETIS_LDFLAGS="$MPI_LDFLAGS -L$PARMETIS_LIB_PATH"
		  LIBS="$LIBS $PARMETIS_LIBS"],[
		  HAVE_PARMETIS="0"
		  AC_MSG_WARN(libparmetis not found!)])
      fi

#      AC_LANG_POP([C++])
      
      # pre-set variable for summary
      with_parmetis="no"
      
      # did it work?
      if test x$HAVE_PARMETIS = x1 ; then
	  AC_SUBST(PARMETIS_LDFLAGS, $PARMETIS_LDFLAGS)
	  AC_SUBST(PARMETIS_LIBS, $PARMETIS_LIBS)
	  AC_SUBST(PARMETIS_CPPFLAGS, $PARMETIS_CPPFLAGS)
	  AC_DEFINE(HAVE_PARMETIS, 1, [Define to 1 if PARMETIS is found])
	  
    # add to global list
	  DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $PARMETIS_LDFLAGS"
	  DUNE_PKG_LIBS="$DUNE_PKG_LIBS $PARMETIS_LIBS"
	  DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $PARMETIS_CPPFLAGS"
	  
    # re-set variable correctly
	  with_parmetis="yes"
      fi 
      
  # end of "no --without-parmetis"
  fi
  
  # tell automake	
  AM_CONDITIONAL(PARMETIS, test x$HAVE_PARMETIS = x1)
  
  # restore variables
  LDFLAGS="$ac_save_LDFLAGS"
  CPPFLAGS="$ac_save_CPPFLAGS"
  LIBS="$ac_save_LIBS"
  
])

dnl Local Variables:
dnl mode: shell-script
dnl End:
