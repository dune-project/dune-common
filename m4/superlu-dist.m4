#! /bin/bash
# $Id$
# searches for Superlu_Dist headers and libs

AC_DEFUN([DUNE_PATH_SUPERLU_DIST],[
	AC_MSG_CHECKING(for SuperLUDist library)
	AC_REQUIRE([AC_PROG_CC])
	AC_REQUIRE([AC_PATH_XTRA])
	AC_REQUIRE([AC_F77_LIBRARY_LDFLAGS])
	AC_REQUIRE([ACX_BLAS])
	AC_REQUIRE([DUNE_MPI])

  #
  # USer hints ...
  #
	my_lib_path=""
	my_include_path=""
	AC_ARG_VAR([SUPERLU_DIST], [SuperLU-Dist library location])
	AC_ARG_WITH([superlu_dist],
	    [AC_HELP_STRING([--with-superlu-dist],[user defined path to SuperLUDist library])],
	    [
		if test -n "$SUPERLU_DIST" ; then
		    AC_MSG_RESULT(yes)
		    with_superlu_dist=$SUPERLU_DIST
		elif test "$withval" != no ; then
		    AC_MSG_RESULT(yes)
		    with_superlu_dist=$withval
		else
		    AC_MSG_RESULT(no)
		fi
		if test "$withval" != no ; then
		    header="$with_superlu_dist/SRC/superlu_ddefs.h"
		    if test -f "$header"; then
			my_include_path="SRC"
		    fi
		fi
		],
	    [
		if test -n "$SUPERLU_DIST" ; then
		    with_superlu_dist=$SUPERLU_DIST
		    AC_MSG_RESULT(yes)
		else
		    with_superlu_dist=/usr/
		    my_include_path=include
		    my_lib_path=lib
		    if test ! -f "$with_superlu_dist/$my_include_path/superlu_ddefs.h" ; then
		#Try to find headers under superlu_dist
			my_include_path=include/superludist
			if test ! -f "$with_superlu_dist/$my_include_path/superlu_ddefs.h" ; then
			    with_superlu_dist=/usr/local/
			    my_include_path=include
			    if test ! -f "$with_superlu_dist/$my_include_path/superlu_ddefs.h" ; then
				my_include_path=include/superludist
				if test ! -f "$with_superlu_dist/$my_include_path/superlu_ddefs.h" ; then
				    with_superlu_dist="no"
				    AC_MSG_RESULT(failed)
				else
				    AC_MSG_RESULT(yes)
				fi
			    else
				AC_MSG_RESULT(yes)
			    fi
			else
			    AC_MSG_RESULT(yes)
			fi
		    else
			AC_MSG_RESULT(yes) 
		    fi
		fi
		])
	
	AC_ARG_VAR([SUPERLU_DIST_LIB], [The static SuperLU-Dist library name])
	AC_ARG_WITH([super_lu_dist_lib],
	    [AC_HELP_STRING([--with-superlu-dist-lib],[The name of the static SuperLUDist library to link to. By default the shared library with the name superlu-mpi is tried])],
	    [
		if test -n "$SUPERLU_DIST_LIB"; then
		    with_spuperlu_dist_lib=$SUPERLU_DIST_LIB
		elif test "$withval" != no ; then
		    with_superlu_dist_lib=$withval
		fi
	    ]
	)

  # store old values
	ac_save_LDFLAGS="$LDFLAGS"
	ac_save_CPPFLAGS="$CPPFLAGS"
	ac_save_LIBS="$LIBS"
	
  ## do nothing if --without-superlu_dist is used
	if test x"$with_superlu_dist" != x"no" ; then
	    
      # defaultpath
	    SUPERLU_DIST_LIB_PATH="$with_superlu_dist$my_lib_path"
	    SUPERLU_DIST_INCLUDE_PATH="$with_superlu_dist$my_include_path"
	    
	    SUPERLU_DIST_LDFLAGS="-L$SUPERLU_DIST_LIB_PATH $MPI_LDFLAGS"
	    
      # set variables so that tests can use them
	    CPPFLAGS="$CPPFLAGS -I$SUPERLU_DIST_INCLUDE_PATH $MPI_CPPFLAGS"
	    
      # check for central header
	    AC_CHECK_HEADER([superlu_ddefs.h],[
		    SUPERLU_DIST_CPPFLAGS="$CPPFLAGS"
		    HAVE_SUPERLU_DIST="1"],[
		    HAVE_SUPERLU_DIST="0"
		    AC_MSG_WARN([superlu_ddefs.h not found in $SUPERLU_DIST_INCLUDE_PATH with $CPPFLAGS])]
	    )

	    SUPERLU_DIST_CPPFLAGS="-I$SUPERLU_DIST_INCLUDE_PATH $MPI_CPPFLAGS"
	    
      # if header is found check for the libs
	    	    
	    if test x$HAVE_SUPERLU_DIST = x1 ; then

		# set variables so that tests can use them
		OLDFLAGS="$LDFLAGS"
		LDFLAGS="$LDFLAGS -L$SUPERLU_DIST_LIB_PATH $MPI_LDFLAGS"
		LIBS="$BLAS_LIBS $LIBS $FLIBS $MPILIBS $MPI_LDFLAGS"

		AC_CHECK_LIB(superlu-mpi, [pdgssvx],[
			SUPERLU_DIST_LIBS="-lsuperlu-mpi $LIBS"
			SUPERLU_DIST_LDFLAGS="$LDFLAGS"
			HAVE_SUPERLU_DIST="1"
			],[
			HAVE_SUPERLU_DIST="0"
			AC_MSG_WARN(libsuperlu-mpi not found)])

		if test "$HAVE_SUPERLU_DIST" = 0; then
		    if test x$SUPERLU_DIST_LIB = x ; then
			SUPERLU_DIST_LIB=superlu_mpi.a
		    fi
		    AC_MSG_CHECKING([static SuperLUDist library $SUPERLU_DIST_LIB in "$SUPERLU_DIST_LIB_PATH"])
		    
		    if test -f "$SUPERLU_DIST_LIB_PATH/$SUPERLU_DIST_LIB"; then
			LIBS="$SUPERLU_DIST_LIB_PATH/$SUPERLU_DIST_LIB $LIBS"
			LDFLAGS="$OLDFLAGS"
			AC_CHECK_FUNC(pdgssvx,
			    [
				SUPERLU_DIST_LIBS="$LIBS"
				SUPERLU_DIST_LDFLAGS="$LDFLAGS"
				HAVE_SUPERLU_DIST="1"
				AC_MSG_RESULT(yes)
				],
			    [ 
				HAVE_SUPERLU_DIST="0"
				AC_MSG_RESULT(failed) 
			    ]
			)
		    else
			HAVE_SUPERLU_DIST="0"
			AC_MSG_RESULT(failed)
		    fi
		fi
	    fi
      # pre-set variable for summary
      #with_superlu_dist="no"
	    
      # did it work?
	    AC_MSG_CHECKING([SuperLUDist in $with_superlu_dist])
	    if test x$HAVE_SUPERLU_DIST = x1 ; then
		AC_SUBST(SUPERLU_DIST_LDFLAGS, $SUPERLU_DIST_LDFLAGS)
		AC_SUBST(SUPERLU_DIST_LIBS, $SUPERLU_DIST_LIBS)
		AC_SUBST(SUPERLU_DIST_CPPFLAGS, $SUPERLU_DIST_CPPFLAGS)
		AC_DEFINE(HAVE_SUPERLU_DIST, 1, [Define to 1 if SUPERLU_DIST is found])
		AC_MSG_RESULT(ok)
		
    # add to global list
		DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $SUPERLU_DIST_LDFLAGS"
		DUNE_PKG_LIBS="$DUNE_PKG_LIBS $SUPERLU_DIST_LIBS"
		DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $SUPERLU_DIST_CPPFLAGS"
		
    # re-set variable correctly
		with_superlu_dist="yes"
	    else
		with_superlu_dist="no"
		AC_MSG_RESULT(failed)
	    fi 
	    
  # end of "no --without-superlu_dist"
	else
	    with_superlu_dist="no"
	fi
	
  # tell automake	
	AM_CONDITIONAL(SUPERLU_DIST, test x$HAVE_SUPERLU_DIST = x1)
	
  # restore variables
	LDFLAGS="$ac_save_LDFLAGS"
	CPPFLAGS="$ac_save_CPPFLAGS"
	LIBS="$ac_save_LIBS"
	
    ]
)

dnl Local Variables:
dnl mode: shell-script
dnl End:
