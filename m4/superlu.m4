#! /bin/bash
# $Id$
# searches for SuperLU headers and libs

AC_DEFUN([DUNE_PATH_SUPERLU],[
	AC_MSG_CHECKING(for SuperLU library)
	AC_REQUIRE([AC_PROG_CC])
	AC_REQUIRE([AC_PATH_XTRA])
	#AC_REQUIRE([AC_F77_LIBRARY_LDFLAGS])
	AC_REQUIRE([ACX_BLAS])

  #
  # User hints ...
  #
	my_lib_path=""
	my_include_path=""
	AC_ARG_VAR([SUPERLU], [SuperLU library location])
	AC_ARG_WITH([superlu],
	    [AC_HELP_STRING([--with-superlu],[user defined path to SuperLU library])],
	    [
		if test -n "$SUPERLU" ; then
		    AC_MSG_RESULT(yes)
		    with_superlu=$SUPERLU
		elif test "$withval" != no ; then
		    AC_MSG_RESULT(yes)
		    with_superlu=$withval
		else
		    AC_MSG_RESULT(no)
		fi
		if test "$withval" != no ; then
		    my_lib_path=""
		    header="$with_superlu/SRC/dsp_defs.h"
		    if test -f "$header"; then
			my_include_path="SRC"
		    fi
		fi
		],
	    [
		if test -n "$SUPERLU" ; then
		    with_superlu=$SUPERLU
		    AC_MSG_RESULT(yes)
		else
		    with_superlu=/usr/
		    my_include_path=include
		    my_lib_path=lib
		    if test ! -f "$with_superlu/$my_include_path/dsp_defs.h" ; then
		#Try to find headers under superludist
			my_include_path=include/superlu
			if test ! -f "$with_superlu/$my_include_path/dsp_defs.h" ; then
			    with_superlu=/usr/local/
			    my_include_path=include
			    if test ! -f "$with_superlu/$my_include_path/dsp_defs.h" ; then
				my_include_path=include/superlu
				if test ! -f "$with_superlu/$my_include_path/dsp_defs.h" ; then
				    with_superlu="no"
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
  
	AC_ARG_VAR([SUPERLU_LIB], [The static SuperLU library name])
	AC_ARG_WITH([super_lu_lib],
	    [AC_HELP_STRING([--with-superlu-lib],[The name of the static SuperLU library to link to. By default the shared library with the name superlu-mpi is tried])],
	    [
		if test -n "$SUPERLU_LIB"; then
		    with_spuperlu_lib=$SUPERLU_LIB
		elif test "$withval" != no ; then
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
	    SUPERLU_LIB_PATH="$with_superlu$my_lib_path"
	    SUPERLU_INCLUDE_PATH="$with_superlu$my_include_path"
	    
	    SUPERLU_LDFLAGS="-L$SUPERLU_LIB_PATH"
	    
      # set variables so that tests can use them
	    CPPFLAGS="$CPPFLAGS -I$SUPERLU_INCLUDE_PATH"
	    
      # check for central header
	    AC_CHECK_HEADER([dsp_defs.h],[
		    SUPERLU_CPPFLAGS="$CPPFLAGS"
		    HAVE_SUPERLU="1"],[
		    HAVE_SUPERLU="0"
		    AC_MSG_WARN([dsp_defs.h not found in $SUPERLU_INCLUDE_PATH with $CPPFLAGS])]
	    )
	    
      # if header is found check for the libs
	    
	    if test x$HAVE_SUPERLU = x1 ; then

		OLDFLAGS="$LDFLAGS"
		LDFLAGS="$LDFLAGS -L$SUPERLU_LIB_PATH"
		LIBS="$BLAS_LIBS $LIBS $FLIBS"

		AC_CHECK_LIB(superlu, [dgssvx],[
			SUPERLU_LIBS="$LIBS"
			SUPERLU_LDFLAGS="$LDFLAGS"
			HAVE_SUPERLU="1"
			],[
			HAVE_SUPERLU="0"
			AC_MSG_WARN(libsuperlu not found)])
		
		if test "$HAVE_SUPERLU" = 0; then
		    if test x$SUPERLU_LIB = x ; then
			SUPERLU_LIB=superlu.a
		    fi
		    AC_MSG_CHECKING([static superlu library superlu.a in "$SUPERLU_LIB_PATH"])
		    if test -f "$SUPERLU_LIB_PATH/superlu.a"; then
			LIBS="$SUPERLU_LIB_PATH/superlu.a $LIBS"
			AC_CHECK_FUNC(dgssvx,
			    [
				SUPERLU_LDFLAGS="$OLDFLAGS"
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
		AC_SUBST(SUPERLU_LDFLAGS, $SUPERLU_LDFLAGS)
		AC_SUBST(SUPERLU_LIBS, $SUPERLU_LIBS)
		AC_SUBST(SUPERLU_CPPFLAGS, $SUPERLU_CPPFLAGS)
		AC_DEFINE(HAVE_SUPERLU, 1, [Define to 1 if SUPERLU is found])
		AC_MSG_RESULT(ok)
		
    # add to global list
		DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $SUPERLU_LDFLAGS"
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
	
    ]
)
    
dnl Local Variables:
dnl mode: shell-script
dnl End:
