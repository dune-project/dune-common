#! /bin/bash
# $Id$
# searches for SuperLU headers and libs

AC_DEFUN([_slu_lib_path],
    [
	my_include_path=include/superlu
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

AC_DEFUN([_slu_search_versions],
    [
	my_slu_header=slu_ddefs.h
	_slu_lib_path($1, "$my_slu_header")
	if test "$my_slu_found" != "yes"; then 
	    my_slu_header="dsp_defs.h"
	    _slu_lib_path($1, "$my_slu_header")
	fi
    ]
)

AC_DEFUN([_slu_search_default],
    [
	with_superlu=/usr
	_slu_search_versions($with_superlu)
	
	if test "$my_slu_found" = "no"; then
	    with_superlu=/usr/local
	    _slu_search_versions($with_superlu)
	fi
    ]
)

AC_DEFUN([DUNE_PATH_SUPERLU],[
	AC_REQUIRE([AC_PROG_CC])
	#AC_REQUIRE([AC_F77_LIBRARY_LDFLAGS])
	AC_REQUIRE([ACX_BLAS])

  #
  # User hints ...
  #
	my_lib_path=""
	my_include_path=""
	AC_ARG_WITH([superlu],
	    [AC_HELP_STRING([--with-superlu],[user defined path to SuperLU library])],
	    [dnl
		if  test "$withval" != no ; then
		    # get absolute path
		    with_superlu=`eval cd $withval 2>&1 && pwd`
		    if test "$withval" = yes; then
		        # Search in default locations
			_slu_search_default
		    else
		        # Search for the headers in the specified location
			with_superlu="$withval"
			_slu_search_versions("$with_superlu")
		    fi
		fi
		],
	    [dnl
		# Search in default locations
		    _slu_search_default
		])
  
	AC_ARG_WITH([super_lu_lib],
	    [AC_HELP_STRING([--with-superlu-lib],[The name of the static SuperLU library to link to. By default the shared library with the name superlu-mpi is tried])],
	    [
		if test "$withval" != no ; then
		    with_superlu_lib=$withval
		fi
	    ]
	)
  # store old values
	ac_save_LDFLAGS="$LDFLAGS"
	ac_save_CPPFLAGS="$CPPFLAGS"
	ac_save_LIBS="$LIBS"
	
  ## do nothing if --without-superlu is used
	if test x"$with_superlu" != x"no" ; then
      # defaultpath
	    SUPERLU_LIB_PATH="$with_superlu/$my_lib_path"
	    SUPERLU_INCLUDE_PATH="$with_superlu/$my_include_path"
	    
	    
      # set variables so that tests can use them
	    CPPFLAGS="$CPPFLAGS -I$SUPERLU_INCLUDE_PATH"
	    
      # check for central header
	    AC_CHECK_HEADER([$my_slu_header],[
		    SUPERLU_CPPFLAGS="$CPPFLAGS"
		    HAVE_SUPERLU="1"],[
		    HAVE_SUPERLU="0"
		    AC_MSG_WARN([$my_slu_header not found in $SUPERLU_INCLUDE_PATH with $CPPFLAGS])]
	    )
	    
      # if header is found check for the libs
	    
	    if test x$HAVE_SUPERLU = x1 ; then
		# if no blas was found, we assume that superlu was compiled with 
		# internal blas
		if test "x$BLAS_LIBS" = "x"; then
		  LIBS="$BLAS_LIBS $LIBS $FLIBS"
		else
		  LIBS="-lblas $LIBS $FLIBS"
		fi
		HAVE_SUPERLU=0

		if test x$with_superlu_lib = x; then
		    AC_CHECK_LIB(superlu, [dgssvx],[
			    SUPERLU_LIBS="$SUPERLU_LIBS -lsuperlu $LIBS"
			    HAVE_SUPERLU="1"
			    ],[
			    HAVE_SUPERLU="0"
			    AC_MSG_WARN(libsuperlu not found)])
		fi
		if test "$HAVE_SUPERLU" = 0; then
		    if test x$with_superlu_lib = x ; then
			with_superlu_lib=superlu.a
		    fi
		    AC_MSG_CHECKING([static superlu library "$with_superlu_lib" in "$SUPERLU_LIB_PATH"])
		    if test -f "$SUPERLU_LIB_PATH/$with_superlu_lib" ; then
			LIBS="$SUPERLU_LIB_PATH/$with_superlu_lib $LIBS"
			AC_CHECK_FUNC(dgssvx,
			    [
				SUPERLU_LIBS="$LIBS"
				HAVE_SUPERLU="1"
				AC_MSG_RESULT(yes)
			    ],
			    [
				HAVE_SUPERLU="0"
				AC_MSG_RESULT(failed)
			    ]
			)
		    else
			HAVE_SUPERLU="0"
			AC_MSG_RESULT(failed)
		    fi
		fi
	    fi
      # pre-set variable for summary
      #with_superlu="no"
      
      # did it work?
	    AC_MSG_CHECKING([SuperLU in $with_superlu])
	    if test x$HAVE_SUPERLU = x1 ; then
		AC_SUBST(SUPERLU_LIBS, $SUPERLU_LIBS)
		AC_SUBST(SUPERLU_CPPFLAGS, $SUPERLU_CPPFLAGS)
		AC_DEFINE(HAVE_SUPERLU, 1, [Define to 1 if SUPERLU is found])
		if test "$my_slu_header" = "slu_ddefs.h"; then
		    AC_DEFINE(SUPERLU_POST_2005_VERSION, 1, [define to 1 if there is  a header slu_ddefs.h in SuperLU])
		fi
		AC_MSG_RESULT(ok)
		
    # add to global list
		DUNE_PKG_LIBS="$DUNE_PKG_LIBS $SUPERLU_LIBS"
		DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $SUPERLU_CPPFLAGS"
		
    # re-set variable correctly
		with_superlu="yes"
	    else
		with_superlu="no"
		AC_MSG_RESULT(failed)
	    fi 
	    
  # end of "no --without-superlu"
	else
	    with_superlu="no"
	fi
	
  # tell automake	
	AM_CONDITIONAL(SUPERLU, test x$HAVE_SUPERLU = x1)
	
  # restore variables
	LDFLAGS="$ac_save_LDFLAGS"
	CPPFLAGS="$ac_save_CPPFLAGS"
	LIBS="$ac_save_LIBS"
	
    DUNE_ADD_SUMMARY_ENTRY([SuperLU],[$with_superlu])

    ]
)
    
dnl Local Variables:
dnl mode: shell-script
dnl End:
