#! /bin/bash
# $Id$
# searches for Superlu_Dist headers and libs
AC_DEFUN([_slu_dist_lib_path],
    [
	my_include_path=include/superludist
	my_lib_path=lib
	my_slu_found=yes
	if test ! -f "$1/$my_include_path/$2" ; then
	    #Try to find headers under superlu
	    my_include_path=include
	    if test ! -f "$with_superlu/$my_include_path/$2" ; then
		my_include_path=SRC
		my_lib_path=""
		if test ! -f "$with_superlu/$my_include_path/$2"; then
		    my_slu_found=no
		fi
	    fi
	fi
    ]
)

#AC_DEFUN([_slu_dist_search_versions],
#    [
#	my_slu_header=slu_ddefs.h
#	_slu_dist_lib_path($1, "$my_slu_header")
#	if test "$my_slu_found" != "yes"; then 
#	    my_slu_header="dsp_defs.h"
#	    _slu_dist_lib_path($1, "$my_slu_header")
#	fi
#    ]
#)

AC_DEFUN([_slu_dist_search_default],
    [
	with_superlu_dist=/usr
	_slu_dist_lib_path($with_superlu_dist, "superlu_ddefs.h")
	
	if test "$my_slu_found" = "no"; then
	    with_superlu_dist=/usr/local
	    _slu_dist_lib_path($with_superlu_dist, "superlu_ddefs.h")
	fi
    ]
)

AC_DEFUN([DUNE_PATH_SUPERLU_DIST],[
	AC_MSG_CHECKING(for SuperLUDist library)
	AC_REQUIRE([AC_PROG_CC])
	AC_REQUIRE([ACX_BLAS])
	AC_REQUIRE([DUNE_MPI])

  #
  # USer hints ...
  #
	my_lib_path=""
	my_include_path=""
	AC_ARG_WITH([superlu_dist],
	    [AC_HELP_STRING([--with-superlu-dist],[user defined path to SuperLUDist library])],
	    [dnl
		if test "$withval" != no ; then
		    # get absolute path
		    with_superlu_dist=`eval cd $withval 2>&1 && pwd`
		    if test "$withval" = yes; then
		        # Search in default locations
			_slu_dist_search_default
		    else
		        # Search for the headers in the specified location
			_slu_dist_lib_path("$with_superlu_dist", "superlu_ddefs.h")
		    fi
		fi
		],
	    [dnl
		# Search in default locations
		    _slu_dist_search_default
		])
	
	AC_ARG_WITH([super_lu_dist_lib],
	    [AC_HELP_STRING([--with-superlu-dist-lib],[The name of the static SuperLUDist library to link to. By default the shared library with the name superlu-mpi is tried])],
	    [
		if test "$withval" != no ; then
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
	    SUPERLU_DIST_LIB_PATH="$with_superlu_dist/$my_lib_path"
	    SUPERLU_DIST_INCLUDE_PATH="$with_superlu_dist/$my_include_path"
	    
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

		AC_CHECK_LIB(superlu-mpi, [pdgssvx],
		    [dnl
			SUPERLU_DIST_LIBS="-lsuperlu-mpi $LIBS"
			SUPERLU_DIST_LDFLAGS="$LDFLAGS"
			HAVE_SUPERLU_DIST="1"
			AC_MSG_RESULT(yes)
			],[dnl
			HAVE_SUPERLU_DIST="0"
			AC_MSG_WARN(libsuperlu-mpi not found)])

		if test "$HAVE_SUPERLU_DIST" = 0; then
		    #check for the static library
		    if test x$with_superlu_dist_lib = x ; then
			with_superlu_dist_lib=superlu_mpi.a
		    fi
		    AC_MSG_CHECKING([static SuperLUDist library $with_superlu_dist_lib in "$SUPERLU_DIST_LIB_PATH"])
		    
		    if test -f "$SUPERLU_DIST_LIB_PATH/$with_superlu_dist_lib"; then
			LIBS="$SUPERLU_DIST_LIB_PATH/$with_superlu_dist_lib $LIBS"
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
			AC_MSG_RESULT(failed)
			HAVE_SUPERLU_DIST="0"
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
