#! /bin/bash
# $Id$
# searches for UG headers and libs

AC_DEFUN([DUNE_PATH_UG],[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PATH_XTRA])
  AC_REQUIRE([DUNE_MPI])

  AC_ARG_WITH(ug,
    AC_HELP_STRING([--with-ug=PATH],[directory with UG inside]))

  # store old values
  ac_save_LDFLAGS="$LDFLAGS"
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_LIBS="$LIBS"
  
  ## do nothing if --without-ug is used
  if test x$with_ug != xno ; then
      
      # is --with-ug=bla used?
      if test "x$with_ug" != x ; then
          if ! test -d $with_ug; then
              AC_MSG_WARN([UG directory $with_ug does not exist!])
          else
              # expand tilde / other stuff
              UGROOT=`cd $with_ug && pwd`
          fi
      fi
      if test "x$UGROOT" = x; then
          # use some default value...
          UGROOT="/usr/local/ug"
      fi
      
      # intermediate variables
      UG_LIB_PATH="$UGROOT/lib"
      UG_INCLUDE_PATH="$UGROOT/include"
      
      UG_LDFLAGS="-L$UG_LIB_PATH"

      # set variables so that tests can use them
      LDFLAGS="$LDFLAGS -L$UG_LIB_PATH"
      CPPFLAGS="$CPPFLAGS -I$UG_INCLUDE_PATH -DENABLE_UG"

      AC_ARG_ENABLE(ug-lgmdomain,
        AC_HELP_STRING([--enable-ug-lgmdomain],[use UG LGM domain (default is standard domain)]))
      if test x"$enable_ug_lgmdomain" = xyes ; then
        UG_LIBS="-lugL2 -lugL3 -ldevS"
      else
        UG_LIBS="-lugS2 -lugS3 -ldevS"
      fi

      # backup CPPFLAGS so I can add an additional flag just for AC_CHECk_HEADER
      CPPFLAGS_BACKUP="$CPPFLAGS"
      CPPFLAGS="$CPPFLAGS -D_2"

      # check for central header
      AC_CHECK_HEADER([$UG_INCLUDE_PATH/gm.h],
      [UG_CPPFLAGS="-I$UG_INCLUDE_PATH -DENABLE_UG"
          HAVE_UG="1"],
      [HAVE_UG="0"
      AC_MSG_WARN([gm.h not found in $UG_INCLUDE_PATH])]
      )

      # pre-set variable for summary
      with_ug="no"
      
      # Restore CPPFLAGS
      CPPFLAGS="$CPPFLAGS_BACKUP"

      # Currently we only check for libug2
      # todo: Check for all the libraries that make up UG
      AC_LANG_PUSH([C++])

      # define LTCXXCOMPILE like it will be defined in the Makefile
      ac_save_CXX="$CXX"
      LTCXXLINK="$srcdir/libtool --tag=CXX --mode=link $CXX $CXXFLAGS $LDFLAGS"
      CXX="$LTCXXLINK"

      if test x$HAVE_UG = x1; then

          # We have found the headers for sure.  Now we reuse the flag to signal
          # whether we found the libs as well.
          HAVE_UG="0"
      
        # If MPI is installed look for the parallel UG
        if test x"$MPI_LDFLAGS" != x""; then
            AC_MSG_CHECKING([UG libraries (parallel)])
            LIBS="$UG_LIBS $MPI_LDFLAGS"
            CPPFLAGS="$UG_CPPFLAGS -DModelP -D_2"
            AC_TRY_LINK(
              [#include "initug.h"
               #include "parallel.h"],
          [int i = UG::D2::InitDDD()],
              [UG_LDFLAGS="$LDFLAGS"
           UG_CPPFLAGS="$UG_CPPFLAGS -DModelP"
           HAVE_UG="1"
           UG_LIBS="$UG_LIBS $MPI_LDFLAGS"
           with_ug="yes (parallel)"
           AC_MSG_RESULT(yes)
              ],
              [AC_MSG_RESULT(no)
           HAVE_UG="0"]
          )
        fi

      # parallel lib not found/does not work?  Let's check for the sequential one
      if test x$HAVE_UG != x1; then
        AC_MSG_CHECKING([UG libraries (sequential)])
        LIBS="$UG_LIBS"
        CPPFLAGS="$UG_CPPFLAGS -D_2"
          AC_TRY_LINK(
              [#include "initug.h"],
          [int i = UG::D2::InitUg(0,0)],
              [UG_LDFLAGS="$LDFLAGS"
           HAVE_UG="1"
               with_ug="yes (sequential)"
           AC_MSG_RESULT(yes)
              ],
              [AC_MSG_RESULT(no)
           HAVE_UG="0"]
          )
      fi

          # Okay.  We have found a UG installation.  But has it been built with --enable-dune?
          # We check this by trying to link to the field int UG::duneMarker, which is there
          # do indicate just this.
          if test x$HAVE_UG = x1 ; then
            AC_MSG_CHECKING([whether UG has been built with --enable-dune])
            AC_TRY_LINK(
                [#define FOR_DUNE
                    #include "dunemarker.h"],
                [int i = UG::duneMarker],
                [#UG_LDFLAGS="$LDFLAGS"
                AC_MSG_RESULT(yes)
                ],
                [AC_MSG_RESULT(no)
                 AC_MSG_WARN([UG has not been built with --enable-dune!])
                HAVE_UG="0"
                with_ug="no"
                ]
                )
          fi


      fi

      CXX="$ac_save_CXX"
      AC_LANG_POP([C++])
      
      # did it work?
      if test x$HAVE_UG = x0 ; then
      # reset flags, so they do not appear in makefiles
      UG_LDFLAGS=""
      UG_LIBS=""
      UG_CPPFLAGS=""
      fi
      if test x$HAVE_UG = x1 ; then
      AC_SUBST(UG_LDFLAGS, $UG_LDFLAGS)
      AC_SUBST(UG_LIBS, $UG_LIBS)
      AC_SUBST(UG_CPPFLAGS, $UG_CPPFLAGS)
      AC_DEFINE(HAVE_UG, ENABLE_UG, 
        [This is only true if UG was found by configure 
         _and_ if the application uses the UG_CPPFLAGS])
      if test x"$enable_ug_lgmdomain" = xyes ; then
        AC_DEFINE(UG_LGMDOMAIN, 1, [use UG LGM domain])
      fi
      
    # add to global list
      DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $UG_LDFLAGS"
      DUNE_PKG_LIBS="$DUNE_PKG_LIBS $UG_LIBS"
      DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $UG_CPPFLAGS"
      
      fi 
      
  # end of "no --without-ug"
  fi
  
  # tell automake   
  AM_CONDITIONAL(UG, test x$HAVE_UG = x1)
  AM_CONDITIONAL(UG_LGMDOMAIN, test x$HAVE_UG = x1 && test x$UG_LGMDOMAIN = x1)
  
  # restore variables
  LDFLAGS="$ac_save_LDFLAGS"
  CPPFLAGS="$ac_save_CPPFLAGS"
  LIBS="$ac_save_LIBS"

  DUNE_ADD_SUMMARY_ENTRY([UG],[$with_ug])
  
])

dnl Local Variables:
dnl mode: shell-script
dnl End:
