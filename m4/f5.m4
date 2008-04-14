# $Id$
# searches for F5-stuff

AC_DEFUN([DUNE_PATH_F5],[
  AC_REQUIRE([DUNE_PATH_HDF5])
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PATH_XTRA])
	
  AC_ARG_WITH(f5,
    AC_HELP_STRING([--with-f5=PATH],[directory with FiberHDF5 inside]),
    # expand tilde / other stuff
    eval with_f5=$with_f5
  )

 # store values
 ac_save_CFLAGS="$CFLAGS"
 ac_save_LDFLAGS="$LDFLAGS"
 ac_save_LIBS="$LIBS"
 LIBS=""

 # start building variables

 # use special F5-lib-path if it's set
 if test x$with_f5 != x ; then
   #  extract absolute path
   if test -d $with_f5; then
     eval with_f5=`cd $with_f5 ; pwd`
   else
     AC_MSG_ERROR([FiberHDF5-directory $with_f5 does not exist])
   fi
   F5ARCH="$with_f5/make/arch"
   AC_MSG_CHECKING([for make/arch])
   if test -x $F5ARCH ; then
     AC_MSG_RESULT([yes])
     LDFLAGS="$HDF5_LDFLAGS -L$with_f5/lib/`$F5ARCH`"
     F5_LDFLAGS="$LDFLAGS"
     CPPFLAGS="$CPPFLAGS -I$with_f5"
   else
     AC_MSG_RESULT([no])
     AC_MSG_ERROR([FiberHDF5 installation incomplete ($F5ARCH missing)])
   fi
 fi

 # test for an arbitrary header
 AC_CHECK_HEADER([F5/F5F.h], 
   [HAVE_F5=1]
    F5_CPPFLAGS="$CPPFLAGS",
   [HAVE_F5=0])

 # test for lib
 if test x$HAVE_F5 = x1 ; then
   AC_CHECK_LIB(F5, H5Fopen,
                [F5_LIBS="-lF5 $HDF5_LIBS"],[HAVE_F5=0], $HDF5_LIBS)
 fi

 # pre-set variable for summary
 with_f5="no"

 # did we succeed?
 if test x$HAVE_F5 = x1 ; then
   AC_SUBST(F5_CPPFLAGS, $F5_CPPFLAGS)
   AC_SUBST(F5_LDFLAGS, $F5_LDFLAGS)
   AC_SUBST(F5_LIBS, $F5_LIBS)
   AC_DEFINE(HAVE_F5, 1, [Define to 1 if f5 was found])

   # proudly show in summary
   with_f5="yes"
 fi

 # also tell automake
 AM_CONDITIONAL(FIBER_HDF5, test x$HAVE_F5 = x1)

 # reset values					    
 CFLAGS="$ac_save_CFLAGS"
 LIBS="$ac_save_LIBS"
 LDFLAGS="$ac_save_LDFLAGS"

 DUNE_ADD_SUMMARY_ENTRY([FiberHDF5],[$with_f5])

])
