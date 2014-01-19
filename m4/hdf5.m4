## -*- autoconf -*-
# $Id$
# searches for HDF5-stuff

# DUNE_PATH_HDF5()
#
# shell variables:
#   with_hdf5
#     no, yes, or "yes (parallel)"
#   with_hdf5_libs
#     empty or something apropriate for LIBS
#   HDF5_CPPFLAGS
#   HDF5_LDFLAGS
#   HDF5_LIBS
#   direct_HDF5_CPPFLAGS
#   direct_HDF5_LDFLAGS
#   direct_HDF5_LIBS
#     same as above, but without variable indirections (e.g. the contents of
#     DUNEMPICPPFLAGS instead of '${DUNEMPICPPFLAGS}')
#   HDF5_PARALLEL
#     1 or undef
#   HAVE_HDF5
#     0 or 1
#
# substitutions:
#   HDF5_CPPFLAGS
#   HDF5_LDFLAGS
#   HDF5_LIBS
#
# defines:
#   HAVE_HDF5
#
# conditionals:
#   HDF5
AC_DEFUN([DUNE_PATH_HDF5],[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PATH_XTRA])
  AC_REQUIRE([DUNE_MPI])

  AC_ARG_WITH(hdf5,
    AC_HELP_STRING([--with-hdf5=PATH],[directory with HDF5 inside]),
    # expand tilde / other stuff
    eval with_hdf5=$with_hdf5
  )
	  
  AC_ARG_WITH(hdf5_libs,
    [AC_HELP_STRING([--with-hdf5-libs=LIBS],[additional libraries needed to link hdf5 programs. Those might be needed if your hdf5 library is static. Possible values are: -lz or -lz -lsz.])],[])

 # store values
 ac_save_CPPFLAGS="$CPPFLAGS"
 ac_save_LDFLAGS="$LDFLAGS"
 ac_save_LIBS="$LIBS"

 # start building variables

 # use special HDF5-lib-path if it's set
 if test x$with_hdf5 != x ; then
   #  extract absolute path
   if test -d $with_hdf5; then
     eval with_hdf5=`cd $with_hdf5 ; pwd`
   else
     AC_MSG_ERROR([HDF5-directory $with_hdf5 does not exist])
   fi
   _dune_hdf5_libpath="-L$with_hdf5/lib"
   _dune_hdf5_incpath="-I$with_hdf5/include"
 else
   _dune_hdf5_libpath=""
   _dune_hdf5_incpath=""
 fi

 CPPFLAGS="$CPPFLAGS $_dune_hdf5_incpath"

 direct_HDF5_CPPFLAGS="$_dune_hdf5_incpath"
 nodep_HDF5_CPPFLAGS="$_dune_hdf5_incpath"
 HDF5_CPPFLAGS="$_dune_hdf5_incpath"
 direct_HDF5_LDFLAGS=""
 nodep_HDF5_LDFLAGS=""
 HDF5_LDFLAGS=""
 direct_HDF5_LIBS=""
 nodep_HDF5_LIBS=""
 HDF5_LIBS=""
 HDF5_PARALLEL=0
 # test if we are parallel
 AC_CHECK_DECL(H5_HAVE_PARALLEL, [dnl
	CPPFLAGS="$CPPFLAGS $DUNEMPICPPFLAGS"
	direct_HDF5_CPPFLAGS="$HDF5_CPPFLAGS $DUNEMPICPPFLAGS"
	HDF5_CPPFLAGS="$HDF5_CPPFLAGS \${DUNEMPICPPFLAGS}"
	LDFLAGS="$LDFLAGS $DUNEMPILDFLAGS"
	direct_HDF5_LDFLAGS="$HDF5_LDFLAGS $DUNEMPILDFLAGS"
	HDF5_LDFLAGS="$HDF5_LDFLAGS \${DUNEMPILDFLAGS}"
	LIBS="$DUNEMPILIBS $LIBS"
	direct_HDF5_LIBS="$DUNEMPILIBS $HDF5_LIBS"
	HDF5_LIBS="\${DUNEMPILIBS} $HDF5_LIBS"
	HDF5_PARALLEL=1],[],[#include"H5pubconf.h"])

 # test for an arbitrary header
 AC_CHECK_HEADER([hdf5.h], 
   [HAVE_HDF5=1],
   [HAVE_HDF5=0])
	
 # Just for the configure check.  In the end, -L has to go into LIBS.
 LDFLAGS="$LDFLAGS $_dune_hdf5_libpath"
 # test for lib
 if test x$HAVE_HDF5 = x1 ; then
   AC_CHECK_LIB([hdf5], [H5open],
     [
       direct_HDF5_LIBS="$_dune_hdf5_libpath -lhdf5 $with_hdf5_libs $direct_HDF5_LIBS"
       nodep_HDF5_LIBS="$_dune_hdf5_libpath -lhdf5 $with_hdf5_libs $nodep_HDF5_LIBS"
       HDF5_LIBS="$_dune_hdf5_libpath -lhdf5 $with_hdf5_libs $HDF5_LIBS"
     ],
     [HAVE_HDF5=0], ["$with_hdf5_libs"])
 fi

 # pre-set variable for summary
 with_hdf5="no"

 # did we succeed?
 if test x$HAVE_HDF5 = x1 ; then
   AC_DEFINE(HAVE_HDF5, 1, [Define to 1 if hdf5 was found])

   # proudly show in summary
   with_hdf5="yes"
   if test x"$HDF5_PARALLEL" = x1; then
     with_hdf5="$with_hdf5 (parallel)"
   fi
 else
   # clear variables
   direct_HDF5_CPPFLAGS=
   nodep_HDF5_CPPFLAGS=
   HDF5_CPPFLAGS=
   direct_HDF5_LDFLAGS=
   nodep_HDF5_LDFLAGS=
   HDF5_LDFLAGS=
   direct_HDF5_LIBS=
   nodep_HDF5_LIBS=
   HDF5_LIBS=
   HDF5_PARALLEL=0
 fi

 AC_SUBST([HDF5_CPPFLAGS])
 AC_SUBST([HDF5_LDFLAGS])
 AC_SUBST([HDF5_LIBS])

 # also tell automake
 AM_CONDITIONAL(HDF5, test x$HAVE_HDF5 = x1)

 # add to global list
 DUNE_ADD_ALL_PKG([HDF5], [$nodep_HDF5_CPPFLAGS],
                  [$nodep_HDF5_LDFLAGS], [$nodep_HDF5_LIBS])

 # reset values					    
 LIBS="$ac_save_LIBS"
 LDFLAGS="$ac_save_LDFLAGS"
 CPPFLAGS="$ac_save_CPPFLAGS"

 DUNE_ADD_SUMMARY_ENTRY([HDF5],[$with_hdf5])

])
