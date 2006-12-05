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
      CPPFLAGS="$CPPFLAGS -I$UG_INCLUDE_PATH"

      AC_ARG_ENABLE(ug-lgmdomain,
        AC_HELP_STRING([--enable-ug-lgmdomain],[use UG LGM domain (default is standard domain)]))
      if test x"$enable_ug_lgmdomain" = xyes ; then
        AC_DEFINE(UG_LGMDOMAIN, 1, [use UG LGM domain])
        # TODO: The lgm libraries contain a circular dependency!
        UG_LIBS="-lug2 -ldomL2 -lug2 -lgg2 -lug3 -ldomL3 -lug3 -lgg3 -ldevS"
      else
        UG_LIBS="-lug2 -ldomS2 -lgg2 -lug3 -ldomS3 -lgg3 -ldevS"
      fi

      # backup CPPFLAGS so I can add an additional flag just for AC_CHECk_HEADER
      CPPFLAGS_BACKUP="$CPPFLAGS"
      CPPFLAGS="$CPPFLAGS -D_2"

      # check for central header
      AC_CHECK_HEADER([$UG_INCLUDE_PATH/gm.h],
	  [UG_CPPFLAGS="-I$UG_INCLUDE_PATH"
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
      if test x$HAVE_UG = x1 ; then

	  CPPFLAGS="$UG_CPPFLAGS"
	  
	  AC_MSG_CHECKING([UG libraries (without MPI)])
	  LIBS="$UG_LIBS"
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

	  # sequential lib not found/does not work?
	  if test x$HAVE_UG != x1 && test x"$MPI_LDFLAGS" != x"" ; then
	    # try again with added MPI-libs
	    UG_LIBS="$UG_LIBS $MPI_LDFLAGS"
	    AC_MSG_CHECKING([UG libraries (with MPI)])
	    LIBS="$UG_LIBS"
            AC_TRY_LINK(
              [#include "initug.h"],
	      [int i = UG::D2::InitUg(0,0)],
              [UG_LDFLAGS="$LDFLAGS"
	       UG_CPPFLAGS="$UG_CPPFLAGS -DModelP"
	       HAVE_UG="1"
               with_ug="yes (parallel)"
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
      AC_LANG_POP([C++])
      
      # did it work?
      if test x$HAVE_UG = x1 ; then
	  AC_SUBST(UG_LDFLAGS, $UG_LDFLAGS)
	  AC_SUBST(UG_LIBS, $UG_LIBS)
	  AC_SUBST(UG_CPPFLAGS, $UG_CPPFLAGS)
	  AC_DEFINE(HAVE_UG, 1, [Define to 1 if UG is found])
	  
    # add to global list
	  DUNE_PKG_LDFLAGS="$DUNE_PKG_LDFLAGS $UG_LDFLAGS"
	  DUNE_PKG_LIBS="$DUNE_PKG_LIBS $UG_LIBS"
	  DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $UG_CPPFLAGS"
	  
      fi 
      
  # end of "no --without-ug"
  fi
  
  # tell automake	
  AM_CONDITIONAL(UG, test x$HAVE_UG = x1)
  
  # restore variables
  LDFLAGS="$ac_save_LDFLAGS"
  CPPFLAGS="$ac_save_CPPFLAGS"
  LIBS="$ac_save_LIBS"
  
])

dnl Local Variables:
dnl mode: shell-script
dnl End:
