# searches for alugrid-headers and libs

AC_DEFUN([DUNE_PATH_ALUGRID],[
  AC_REQUIRE([AC_PROG_CXX])

  AC_ARG_WITH(alugrid,
    AC_HELP_STRING([--with-alugrid=PATH],[directory with ALU3d-Grid inside]))
# do not use alugrid debug lib 

# store old values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"
# LIBS=""

## do nothing if no --with-alugrid was supplied
if test x$with_alugrid != x && test x$with_alugrid != xno ; then

  if test x$with_alugrid == xyes ; then
    AC_MSG_ERROR([You have to provide a directory --with-alugrid=PATH])
  fi

  # is --with-alugrid=bla used?
  if test "x$with_alugrid" != x ; then
	if ! test -d $with_alugrid; then
        AC_MSG_WARN([ALUGrid directory $with_alugrid does not exist])
	else
    # expand tilde / other stuff
		ALUGRIDROOT=`cd $with_alugrid && pwd`

    ALUGRID_VERSIONCHECK=$ALUGRIDROOT/bin/alugridversion
    ## check version number 
    NEEDEDALUGRID_VERSION=1.1

    echo -n "checking ALUGrid version >= $NEEDEDALUGRID_VERSION... " 
    if test -f $ALUGRID_VERSIONCHECK; then 
      ALUGRID_VERSION=`$ALUGRID_VERSIONCHECK -c $NEEDEDALUGRID_VERSION`
      if test "x$ALUGRID_VERSION" != "x-1"; then 
        ALUGRID_VERSIONNO=`$ALUGRID_VERSIONCHECK -v`
        echo "yes (ALUGrid-$ALUGRID_VERSIONNO)"
      else 
        AC_MSG_ERROR([ALUGrid version is too old!])
      fi 
      
    else 
      AC_MSG_WARN([Couldn't find ALUGrid version checker! ALUGrid version too old!])
    fi   
	fi
  fi
  if test "x$ALUGRIDROOT" = x; then
    # use some default value...
    ALUGRIDROOT="/usr/local/alugrid"
  fi

  ALUGRID_LIB_PATH="$ALUGRIDROOT/lib"
  ALUGRID_INCLUDE_PATH="$ALUGRIDROOT/include"

  # set variables so that tests can use them
  REM_CPPFLAGS=$CPPFLAGS

  LDFLAGS="$LDFLAGS -L$ALUGRID_LIB_PATH"
  ALU3D_INC_FLAG="-I$ALUGRID_INCLUDE_PATH -I$ALUGRID_INCLUDE_PATH/serial -I$ALUGRID_INCLUDE_PATH/duneinterface -DENABLE_ALUGRID"
  CPPFLAGS="$CPPFLAGS $ALU3D_INC_FLAG"

  # check for header
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADERS([alugrid_serial.h], 
     [ALUGRID_CPPFLAGS="$ALU3D_INC_FLAG"
    HAVE_ALUGRID="1"],
    AC_MSG_WARN([alugrid_serial.h not found in $ALUGRID_INCLUDE_PATH]))
   
  ALU3D_INC_FLAG_PARA="-I$ALUGRID_INCLUDE_PATH/parallel"
  CPPFLAGS="$CPPFLAGS $ALU3D_INC_FLAG_PARA $MPI_CPPFLAGS"
  # check for parallel header 
  AC_CHECK_HEADERS([alugrid_parallel.h], 
     [ALUGRID_CPPFLAGS="$ALU3D_INC_FLAG $ALU3D_INC_FLAG_PARA"
    HAVE_ALUGRID="1"],
    AC_MSG_WARN([alugrid_parallel.h not found in $ALUGRID_INCLUDE_PATH]))

  CPPFLAGS="$REM_CPPFLAGS"
  REM_CPPFLAGS=

  REM_LDFLAGS=$LDFLAGS

  # if header is found...
  if test x$HAVE_ALUGRID = x1 ; then
    AC_CHECK_LIB(alugrid,[malloc],
    [ALUGRID_LIBS="-lalugrid"
           ALUGRID_LDFLAGS="-L$ALUGRID_LIB_PATH"
           LIBS="$LIBS $ALUGRID_LIBS"],
	  [HAVE_ALUGRID="0"
	  AC_MSG_WARN(libalugrid not found!)])
  fi

  LDFLAGS=$REM_LDFLAGS
  AC_LANG_POP

## end of alugrid check (--without wasn't set)
fi

# survived all tests?
if test x$HAVE_ALUGRID = x1 ; then
  AC_SUBST(ALUGRID_LIBS, $ALUGRID_LIBS)
  AC_SUBST(ALUGRID_LDFLAGS, $ALUGRID_LDFLAGS)
  AC_SUBST(ALUGRID_CPPFLAGS, $ALUGRID_CPPFLAGS)
  AC_DEFINE(HAVE_ALUGRID, ENABLE_ALUGRID,
    [This is only true if alugrid-library was found by configure 
     _and_ if the application uses the ALUGRID_CPPFLAGS])

  # add to global list
  DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $ALUGRID_LDFLAGS"
  DUNE_PKG_LIBS="$DUNE_PKG_LIBS $ALUGRID_LIBS"
  DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $ALUGRID_CPPFLAGS"

  # set variable for summary
  with_alugrid="yes"
else
  AC_SUBST(ALUGRID_LIBS, "")
  AC_SUBST(ALUGRID_LDFLAGS, "")
  AC_SUBST(ALUGRID_CPPFLAGS, "")

  # set variable for summary
  with_alugrid="no"
fi
  
# also tell automake
AM_CONDITIONAL(ALUGRID, test x$HAVE_ALUGRID = x1)

# reset old values
LIBS="$ac_save_LIBS"
CPPFLAGS="$ac_save_CPPFLAGS"
LDFLAGS="$ac_save_LDFLAGS"

])
