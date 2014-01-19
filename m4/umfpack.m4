# courtesy of the dune-fem maintainers

AC_DEFUN([DUNE_PATH_UMFPACK],[
  AC_REQUIRE([AC_PROG_CC])

  AC_ARG_WITH(umfpack,
    AC_HELP_STRING([--with-umfpack=PATH],[directory with UMFPACK inside]))
  AC_ARG_WITH(umfpack-includedir,
    AC_HELP_STRING([--with-umfpack-includedir=PATH],[directory with UMFPACK headers inside]))
  AC_ARG_WITH(umfpack-libdir,
    AC_HELP_STRING([--with-umfpack-libdir=PATH],[directory with UMFPACK libraries inside]))

# store old values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"

UMFPACKYES=0
## do nothing if no --with-umfpack was supplied
if test x$with_umfpack != x && test x$with_umfpack != xno ; then
  UMFPACKYES=1
fi
if test x$with_umfpack_includedir != x && test x$with_umfpack_includedir != xno ; then
  UMFPACKYES=1
fi
if test x$with_umfpack_libdir != x && test x$with_umfpack_libdir != xno ; then
  UMFPACKYES=1
fi

if test x$UMFPACKYES = x1 ; then

  # is --with-umfpack=bla used?
  if test "x$with_umfpack" != x ; then
    UMFPACKROOT=`cd $with_umfpack && pwd`
	  if ! test -d $UMFPACKROOT;  then
      AC_MSG_WARN([UMFPACK directory $with_umfpack does not exist])
  	fi

    if test "x$UMFPACKROOT" = x; then
      # use some default value...
      UMFPACKROOT="/usr/local/umfpack"
    fi

    UMFAMD_LIB_PATH="$UMFPACKROOT/AMD/Lib"
    UMFPACK_LIB_PATH="$UMFPACKROOT/UMFPACK/Lib"
    UMFPACK_INCLUDE_PATH="$UMFPACKROOT/UMFPACK/Include"
  else 
    if test "x$with_umfpack_includedir" != x ; then 
      UMFPACK_INCLUDE_PATH=`cd $with_umfpack_includedir && pwd`
      if ! test -d $UMFPACK_INCLUDE_PATH;  then
        AC_MSG_WARN([UMFPACK directory $with_umfpack_includedir does not exist])
      fi
    fi
    if test "x$with_umfpack_libdir" != x ; then 
      UMFPACK_LIB_PATH=`cd $with_umfpack_libdir && pwd`
      if ! test -d $UMFPACK_LIB_PATH;  then
        AC_MSG_WARN([UMFPACK directory $with_umfpack_libdir does not exist])
      fi
    fi
    UMFAMD_LIB_PATH=$UMFPACK_LIB_PATH
  fi

  # set variables so that tests can use them
  REM_CPPFLAGS=$CPPFLAGS

  LDFLAGS="$LDFLAGS -L$UMFPACK_LIB_PATH -L$UMFAMD_LIB_PATH"
  UMFPACK_INC_FLAG="-I$UMFPACK_INCLUDE_PATH -I$UMFPACKROOT/UFconfig -I$UMFPACKROOT/AMD/Include -I$UMFPACKROOT/SuiteSparse_config -DENABLE_UMFPACK=1"
  CPPFLAGS="$CPPFLAGS $UMFPACK_INC_FLAG $MPI_CPPFLAGS"

  # check for header
  AC_LANG_PUSH([C])
  AC_CHECK_HEADERS([umfpack.h], 
     [UMFPACK_CPPFLAGS="$UMFPACK_INC_FLAG"
    HAVE_UMFPACK="1"],
    AC_MSG_WARN([umfpack.h not found in $UMFPACK_INCLUDE_PATH]))
   
  CPPFLAGS="$REM_CPPFLAGS"
  REM_CPPFLAGS=

  REM_LDFLAGS=$LDFLAGS

  # if header is found...
  if test x$HAVE_UMFPACK = x1 ; then
    AC_CHECK_LIB(umfpack,[main],
    [UMFPACK_LIBS="-lumfpack"
           UMFPACK_LDFLAGS="-L$UMFPACK_LIB_PATH"],
	  [HAVE_UMFPACK="0"
	  AC_MSG_WARN(libumfpack not found!)])
  fi

  # if lib is found...
  if test x$HAVE_UMFPACK = x1 ; then
    AC_CHECK_LIB(amd,[main],
    [UMFPACK_LIBS="$UMFPACK_LIBS -lamd"
           UMFPACK_LDFLAGS="$UMFPACK_LDFLAGS -L$UMFAMD_LIB_PATH"
           LIBS="$LIBS $UMFPACK_LIBS"],
	  [HAVE_UMFPACK="0"
	  AC_MSG_WARN(libamd not found!)])
  fi

  LDFLAGS=$REM_LDFLAGS
  AC_LANG_POP

## end of umfpack check (--without wasn't set)
fi

# survived all tests?
if test x$HAVE_UMFPACK = x1 ; then
  AC_SUBST(UMFPACK_LIBS, $UMFPACK_LIBS)
  AC_SUBST(UMFPACK_LDFLAGS, $UMFPACK_LDFLAGS)
  AC_SUBST(UMFPACK_CPPFLAGS, $UMFPACK_CPPFLAGS)
  AC_DEFINE(HAVE_UMFPACK, ENABLE_UMFPACK,
    [This is only true if umfpack-library was found by configure 
     _and_ if the application uses the UMFPACK_CPPFLAGS])

  # add to global list
  DUNE_ADD_ALL_PKG([UMFPACK], [\${UMFPACK_CPPFLAGS}],
                   [\${UMFPACK_LDFLAGS}], [\${UMFPACK_LIBS}])

  # set variable for summary
  with_umfpack="yes"

else
  AC_SUBST(UMFPACK_LIBS, "")
  AC_SUBST(UMFPACK_LDFLAGS, "")
  AC_SUBST(UMFPACK_CPPFLAGS, "")

  # set variable for summary
  with_umfpack="no"
fi
  
# also tell automake
AM_CONDITIONAL(UMFPACK, test x$HAVE_UMFPACK = x1)

# reset old values
LIBS="$ac_save_LIBS"
CPPFLAGS="$ac_save_CPPFLAGS"
LDFLAGS="$ac_save_LDFLAGS"

  DUNE_ADD_SUMMARY_ENTRY([UMFPACK],[$with_umfpack])
])
