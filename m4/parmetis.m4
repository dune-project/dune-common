dnl -*- mode: autoconf; tab-width: 8; indent-tabs-mode: nil; -*-
dnl vi: set et ts=8 sw=2 sts=2:
# $Id$
# searches for ParMetis headers and libs

AC_DEFUN([DUNE_PATH_PARMETIS],[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PATH_XTRA])
  AC_REQUIRE([DUNE_MPI])

  # default locations
  PARMETIS_DEFAULT="/usr /usr/local"
  INCLUDE_DEFAULT="include include/parmetis"

  #
  # User hints ...
  #
  AC_ARG_VAR([PARMETIS], [ParMETIS library location])
  AC_ARG_WITH([parmetis],
    [AC_HELP_STRING([--with-parmetis],[user defined path to ParMETIS library])],
    [
      # --with-parmetis supersedes $PARMETIS
      PARMETIS="$withval"
    ])
  # set PARMETIS to the usual locations, in case it is not defined
  if test -n "$PARMETIS" ; then PARMETIS_DEFAULT=""; fi
  with_parmetis=""
  include_path=""
  lib_path=""
  # check for parmtirs include path
  for p in "$PARMETIS" $PARMETIS_DEFAULT; do
    for i in $INCLUDE_DEFAULT; do
#      AC_MSG_NOTICE( ... checking for $p/$i )
      if test -f $p/$i/parmetis.h ; then
#        AC_MSG_NOTICE( ... found $p/$i/parmetis.h )
        with_parmetis="$p"
        include_path="$i"
        lib_path=lib
        break 2;
      fi
    done
  done
  # report test result
  if test -n "$with_parmetis" ; then
    # defaultpath
    PARMETIS_LIB_PATH="$with_parmetis/$lib_path"
    PARMETIS_INCLUDE_PATH="$with_parmetis/$include_path"

    AC_MSG_RESULT(yes)
  else
    if test -n "$PARMETIS" ; then
      AC_MSG_ERROR(no)
    else
      AC_MSG_RESULT(no)
    fi
  fi

  AC_MSG_NOTICE( PARMETIS: checking for $p/$i )
  AC_ARG_WITH([parmetis-lib],
    [AC_HELP_STRING([--with-parmetis-lib],  [name of the parmetis libraries (default is parmetis)])],
    ,[with_parmetis_lib=parmetis])
  AC_ARG_WITH([metis-lib],
    [AC_HELP_STRING([--with-metis-lib],  [name of the metis libraries (default is metis)])],
    ,[with_metis_lib=metis])

  #echo with_parmetis=$with_parmetis
  # store old values
  ac_save_LDFLAGS="$LDFLAGS"
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_LIBS="$LIBS"

  # call IMDX_LIB_METIS directly and not via AC_REQUIRE
  # because AC_REQUIRE support not allow parameters
  # without any parameters a missing METIS would halt configure
  IMMDX_LIB_METIS(,[true])

  ## do nothing if --without-parmetis is used
  if test x"$with_mpi" != x"no" && test x"$with_parmetis" != x"no" ; then

      PARMETIS_LIBS="-L$PARMETIS_LIB_PATH -l$with_metis_lib $DUNEMPILIBS -lm"
      PARMETIS_LDFLAGS="$DUNEMPILDFLAGS"

      # set variables so that tests can use them
      CPPFLAGS="$CPPFLAGS -I$PARMETIS_INCLUDE_PATH $METIS_INCLUDE $DUNEMPICPPFLAGS"

      # check for central header
      AC_CHECK_HEADER([parmetis.h],[
	      PARMETIS_CPPFLAGS="-I$PARMETIS_INCLUDE_PATH $METIS_INCLUDE"
	      HAVE_PARMETIS="1"],[
	      HAVE_PARMETIS="0"
	      AC_MSG_WARN([parmetis.h not found in $PARMETIS_INCLUDE_PATH with $CPPFLAGS])]
      )

      PARMETIS_CPPFLAGS="${DUNEMPICPPFLAGS} ${PARMETIS_CPPFLAGS} -DENABLE_PARMETIS=1"

#      AC_LANG_PUSH([C++])

      # if header is found check for the libs

      LIBS="$DUNEMPILIBS -lm $LIBS"

      if test x$HAVE_PARMETIS = x1 ; then
	  DUNE_CHECK_LIB_EXT([$PARMETIS_LIB_PATH], [$with_metis_lib], [metis_partgraphkway],
              [
		  PARMETIS_LIBS="-L$PARMETIS_LIB_PATH -l$with_metis_lib $METIS_LDFLAGS $DUNEMPILIBS -lm"
		  LIBS="$PARMETIS_LIBS $ac_save_LIBS"
              ],[
		  HAVE_PARMETIS="0"
		  AC_MSG_WARN(libmetis not found!)
              ])
      fi

      if test x$HAVE_PARMETIS = x1 ; then
	  DUNE_CHECK_LIB_EXT([$PARMETIS_LIB_PATH], [$with_parmetis_lib], [parmetis_v3_partkway],
              [
		  PARMETIS_LIBS="-L$PARMETIS_LIB_PATH -l$with_parmetis_lib $PARMETIS_LIBS"
              ],[
		  HAVE_PARMETIS="0"
		  AC_MSG_WARN(libparmetis not found!)
              ])
      fi

#      AC_LANG_POP([C++])

      # pre-set variable for summary
      #with_parmetis="no"

      # did it work?
      AC_MSG_CHECKING(ParMETIS in $with_parmetis)
      if test x$HAVE_PARMETIS = x1 ; then
	  AC_SUBST(PARMETIS_LIBS, $PARMETIS_LIBS)
	  AC_SUBST(PARMETIS_LDFLAGS, $PARMETIS_LDFLAGS)
	  AC_SUBST(PARMETIS_CPPFLAGS, $PARMETIS_CPPFLAGS)
	  AC_DEFINE(HAVE_PARMETIS,ENABLE_PARMETIS,[Define if you have the Parmetis library.
		  This is only true if MPI was found by configure
		  _and_ if the application uses the PARMETIS_CPPFLAGS])
	  AC_MSG_RESULT(ok)

          # add to global list
          DUNE_ADD_ALL_PKG([PARMETIS], [\${PARMETIS_CPPFLAGS}],
                           [\${PARMETIS_LDFLAGS}], [\${PARMETIS_LIBS}])

          # re-set variable correctly
	  with_parmetis="yes"
      else
          AC_SUBST(PARMETIS_LIBS, "")
          AC_SUBST(PARMETIS_LDFLAGS, "")
          AC_SUBST(PARMETIS_CPPFLAGS, "")
	  with_parmetis="no"
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
