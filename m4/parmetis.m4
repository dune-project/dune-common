#! /bin/bash
# $Id$
# searches for ParMetis headers and libs

AC_DEFUN([DUNE_PATH_PARMETIS],[
  AC_MSG_CHECKING(for ParMETIS library)
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PATH_XTRA])
  AC_REQUIRE([DUNE_MPI])

  #
  # USer hints ...
  #
  AC_ARG_VAR([PARMETIS], [ParMETIS library location])
  AC_ARG_WITH([parmetis],
    [AC_HELP_STRING([--with-parmetis],[user defined path to ParMETIS library])],
    [
	if test -n "$PARMETIS" ; then
	    AC_MSG_RESULT(yes)
	    with_parmetis=$PARMETIS
	elif test "$withval" != no ; then
	    AC_MSG_RESULT(yes)
	    with_parmetis=$withval
	else
	    AC_MSG_RESULT(no)
	fi
	],
    [
	if test -n "$PARMETIS" ; then
	    with_parmetis=$PARMETIS
	    AC_MSG_RESULT(yes)
	else
	    with_parmetis=/usr/
	    include_path=include
	    lib_path=lib
	    if test ! -f "$with_parmetis/$include_path/parmetis.h" ; then
		with_parmetis=/usr/local/
		if test ! -f "$with_metis/$include_path/parmetis.h" ; then
		    with_parmetis="no"
		    AC_MSG_RESULT(failed)
		else
		    AC_MSG_RESULT(yes)
		fi
	    else
		AC_MSG_RESULT(yes)
	    fi
	fi
	])
  

  # store old values
  ac_save_LDFLAGS="$LDFLAGS"
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_LIBS="$LIBS"
  
  ## do nothing if --without-parmetis is used
  if test x"$MPI_LDFLAGS" != x"" && test x"$with_parmetis" != x"no" ; then
          
      # defaultpath
      PARMETIS_LIB_PATH="$with_parmetis$lib_path"
      PARMETIS_INCLUDE_PATH="$with_parmetis$lib_path"
                  
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

      LIBS="$MPILIBS $MPI_LDFLAGS"
      
      if test x$HAVE_PARMETIS = x1 ; then
	  AC_CHECK_LIB(metis, [metis_partgraphkway],[
		  PARMETIS_LIBS=" -lmetis"
		  PARMETIS_LDFLAGS="-L$PARMETIS_LIB_PATH"
		  LIBS="$LIBS -lmetis"],[
		  HAVE_PARMETIS="0"
		  AC_MSG_WARN(libparmetis not found!)])
      fi

      if test x$HAVE_PARMETIS = x1 ; then
	  AC_CHECK_LIB(parmetis, [parmetis_v3_partkway],[
		  PARMETIS_LIBS="-lparmetis -lmetis $MPILIBS $MPI_LDFLAGS"
		  PARMETIS_LDFLAGS="$MPI_LDFLAGS -L$PARMETIS_LIB_PATH"
		  HAVE_PARMETIS="1"],[
		  HAVE_PARMETIS="0"
		  AC_MSG_WARN(libparmetis not found!)])
      fi

#      AC_LANG_POP([C++])
      
      # pre-set variable for summary
      #with_parmetis="no"
      
      # did it work?
      AC_MSG_CHECKING(ParMETIS in $with_parmetis)
      if test x$HAVE_PARMETIS = x1 ; then
	  AC_SUBST(PARMETIS_LDFLAGS, $PARMETIS_LDFLAGS)
	  AC_SUBST(PARMETIS_LIBS, $PARMETIS_LIBS)
	  AC_SUBST(PARMETIS_CPPFLAGS, $PARMETIS_CPPFLAGS)
	  AC_DEFINE(HAVE_PARMETIS, 1, [Define to 1 if PARMETIS is found])
	  AC_MSG_RESULT(ok)
	  
    # add to global list
	  DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $PARMETIS_LDFLAGS"
	  DUNE_PKG_LIBS="$DUNE_PKG_LIBS $PARMETIS_LIBS"
	  DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $PARMETIS_CPPFLAGS"
	  
    # re-set variable correctly
	  with_parmetis="yes"
      else
	  AC_MSG_RESULT(failed)
      fi 
      
  # end of "no --without-parmetis"
  else
  	with_parmetis="no"
  fi
 
  # tell automake	
  AM_CONDITIONAL(PARMETIS, test x$HAVE_PARMETIS = x1)
  
  # restore variables
  LDFLAGS="$ac_save_LDFLAGS"
  CPPFLAGS="$ac_save_CPPFLAGS"
  LIBS="$ac_save_LIBS"
  
  DUNE_ADD_SUMMARY_ENTRY([ParMETIS],[$with_parmetis])

])

dnl Local Variables:
dnl mode: shell-script
dnl End:
