# $Id$
# searches for alu3dgrid-headers and libs

AC_DEFUN([DUNE_PATH_ALU3DGRID],[
  AC_REQUIRE([AC_PROG_CXX])

  AC_ARG_WITH(alu3dgrid,
    AC_HELP_STRING([--with-alu3dgrid=PATH],[directory with ALU3d-Grid inside]))
# do not use alu3dgrid debug lib 

# store old values
ac_save_LDFLAGS="$LDFLAGS"
ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"
# LIBS=""

## do nothing if no --with-alu3dgrid was supplied
if test x$with_alu3dgrid != x && test x$with_alu3dgrid != xno ; then

  if test x$with_alu3dgrid == xyes ; then
    AC_MSG_ERROR([You have to provide a directory --with-alu3dgrid=PATH])
  fi

  if test -d $with_alu3dgrid; then
    # expand tilde / other stuff
    ALU3DGRIDROOT=`cd $with_alu3dgrid && pwd`
  else
    AC_MSG_ERROR([Path $with_alu3dgrid supplied for --with-alu3dgrid does not exist!])
  fi

  ALU3DGRID_LIB_PATH="$ALU3DGRIDROOT/lib"
  ALU3DGRID_INCLUDE_PATH="$ALU3DGRIDROOT/include"

  # set variables so that tests can use them
  REM_CPPFLAGS=$CPPFLAGS

  LDFLAGS="$LDFLAGS -static -L$ALU3DGRID_LIB_PATH"
  ALU3D_INC_FLAG="-I$ALU3DGRID_INCLUDE_PATH -I$ALU3DGRID_INCLUDE_PATH/serial -I$ALU3DGRID_INCLUDE_PATH/duneinterface"
  CPPFLAGS="$CPPFLAGS $ALU3D_INC_FLAG"

  # check for header
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADERS([alu3dgrid_serial.h], 
     [ALU3DGRID_CPPFLAGS="$ALU3D_INC_FLAG"
    HAVE_ALU3DGRID="1"],
    AC_MSG_WARN([alu3dgrid_serial.h not found in $ALU3DGRID_INCLUDE_PATH]))
   
  ALU3D_INC_FLAG_PARA="-I$ALU3DGRID_INCLUDE_PATH/parallel $MPI_CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $ALU3D_INC_FLAG_PARA"
  # check for parallel header 
  AC_CHECK_HEADERS([alu3dgrid_parallel.h], 
     [ALU3DGRID_CPPFLAGS="$ALU3D_INC_FLAG $ALU3D_INC_FLAG_PARA"
    HAVE_ALU3DGRID="1"],
    AC_MSG_WARN([alu3dgrid_parallel.h not found in $ALU3DGRID_INCLUDE_PATH]))

  CPPFLAGS="$REM_CPPFLAGS"
  REM_CPPFLAGS=

  REM_LDFLAGS=$LDFLAGS

  # if header is found...
  if test x$HAVE_ALU3DGRID = x1 ; then
    AC_CHECK_LIB(alu3dgrid,[main],
    [ALU3DGRID_LIBS="-lalu3dgrid"
           ALU3DGRID_LDFLAGS="-L$ALU3DGRID_LIB_PATH"
           LIBS="$LIBS $ALU3DGRID_LIBS"],
	  [HAVE_ALU3DGRID="0"
	  AC_MSG_WARN(libalu3dgrid not found!)])
  fi

  LDFLAGS=$REM_LDFLAGS
  AC_LANG_POP

## end of alu3dgrid check (--without wasn't set)
fi

# survived all tests?
if test x$HAVE_ALU3DGRID = x1 ; then
  AC_SUBST(ALU3DGRID_LIBS, $ALU3DGRID_LIBS)
  AC_SUBST(ALU3DGRID_LDFLAGS, $ALU3DGRID_LDFLAGS)
  AC_SUBST(ALU3DGRID_CPPFLAGS, $ALU3DGRID_CPPFLAGS)
  AC_DEFINE(HAVE_ALU3DGRID, 1, [Define to 1 if alu3dgrid-library is found])

  # add to global list
  DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $ALU3DGRID_LDFLAGS"
  DUNE_PKG_LIBS="$DUNE_PKG_LIBS $ALU3DGRID_LIBS"
  DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $ALU3DGRID_CPPFLAGS"

  # set variable for summary
  with_alu3dgrid="yes"
else
  AC_SUBST(ALU3DGRID_LIBS, "")
  AC_SUBST(ALU3DGRID_LDFLAGS, "")
  AC_SUBST(ALU3DGRID_CPPFLAGS, "")

  # set variable for summary
  with_alu3dgrid="no"
fi
  
# also tell automake
AM_CONDITIONAL(ALU3DGRID, test x$HAVE_ALU3DGRID = x1)

# reset old values
LIBS="$ac_save_LIBS"
CPPFLAGS="$ac_save_CPPFLAGS"
LDFLAGS="$ac_save_LDFLAGS"

])
