# check for auxiliary tools so that it's not fatal if they're missing
AC_DEFUN([DUNE_DOC_PROGS],[
  AC_CHECK_PROGS([DOXYGEN], [doxygen], [true])
  AM_CONDITIONAL([DOXYGEN], [test "x$DOXYGEN" != xtrue])
  AC_CHECK_PROGS([TEX], [latex], [true])
  AC_CHECK_PROGS([BIBTEX], [bibtex], [true])
  AC_CHECK_PROGS([DVIPDF], [dvipdf], [true])
  AC_CHECK_PROGS([DVIPS], [dvips], [true])
  AC_CHECK_PROGS([WML], [wml], [true])
  AM_CONDITIONAL([WML], [test "x$WML" != xtrue])
  AC_CHECK_PROGS([DOT], [dot], [true])
  AM_CONDITIONAL([DOT], [test "x$DOT" != xtrue])
  AC_CHECK_PROGS([PERL], [perl], [true])
  AC_REQUIRE([DUNE_INKSCAPE])
  AC_CHECK_PROGS([CONVERT], [convert], [true])
  AM_CONDITIONAL([CONVERT], [test "x$CONVERT" != xtrue])
])

# searches for documentation 
AC_DEFUN([DUNE_DOCUMENTATION],[

  AC_REQUIRE([DUNE_OFFICIAL_TARBALLS])
  AC_REQUIRE([DUNE_DOC_PROGS])
  AC_REQUIRE([DUNE_WEB])

  # official tarballs require all documentation programs
  AS_IF([test "x$enable_officialtarballs" = "xyes"],[
    AS_IF([test "x$DOXYGEN" = "xtrue"],[
      AC_MSG_ERROR([doxygen is missing. This program is required for official tarballs!])])
    AS_IF([test "x$LATEX" = "xtrue"],[
      AC_MSG_ERROR([latex is missing. This program is required for official tarballs!])])
    AS_IF([test "x$BIBTEX" = "xtrue"],[
      AC_MSG_ERROR([bibtex is missing. This program is required for official tarballs!])])
    AS_IF([test "x$DVIPDF" = "xtrue"],[
      AC_MSG_ERROR([dvipdf is missing. This program is required for official tarballs!])])
    AS_IF([test "x$DVIPS" = "xtrue"],[
      AC_MSG_ERROR([dvips is missing. This program is required for official tarballs!])])
    AS_IF([test "x$DOT" = "xtrue"],[
      AC_MSG_ERROR([dot is missing. This program is required for official tarballs!])])
    AS_IF([test "x$PERL" = "xtrue"],[
      AC_MSG_ERROR([perl is missing. This program is required for official tarballs!])])
    AS_IF([test "x$CONVERT" = "xtrue"],[
      AC_MSG_ERROR([convert is missing. This program is required for official tarballs!])])
    AS_IF([test "x$INKSCAPE" = xfalse || test "x$INKSCAPE" = xno],[
      AC_MSG_ERROR([inkscape is missing. This program is required for official tarballs!])])
  ])

  # disable documentation rules
  AC_ARG_ENABLE(documentation,
   AS_HELP_STRING([--disable-documentation],[don\'t generate docs, speeds up the build]))
  AS_IF([test "x$enable_officialtarballs" = "xyes"],[
    # official tarballs require documentation
    AS_IF([test "x$enable_documentation" = "xno"],[
      AC_MSG_WARN([official tarballs require documentation, ignoring '--disable-documentation'!])
      enable_documentation="yes"
    ])
  ])
  AM_CONDITIONAL(BUILD_DOCS, [test "x$enable_documentation" != "xno"])

])

AC_DEFUN([DUNE_WEB],
[
  AC_REQUIRE([DUNE_OFFICIAL_TARBALLS])
  AC_REQUIRE([DUNE_DOC_PROGS])

  # special variable to include the documentation into the website
  AC_ARG_WITH(duneweb,
    AS_HELP_STRING([--with-duneweb=PATH],[Only needed for website-generation, path to checked out version of dune-web]))

  AS_IF([test -n "$with_duneweb"],[

    # disable dune-web in official tarball mode
    AS_IF([test "x$enable_officialtarballs" = "xyes"],[
      AC_MSG_WARN([ignoring dune-web... official tarballs enabled])
      with_duneweb="no"
    ])
   
    # disable dune-web if wml or doxygen is missing
    AS_IF([test "x$WML" = "xtrue"],[
      AC_MSG_WARN([ignoring dune-web... wml missing])
      with_duneweb="no"
    ])
    AS_IF([test "x$DOXYGEN" = "xtrue"],[
      AC_MSG_WARN([ignoring dune-web... doxygen missing])
      with_duneweb="no"
    ])

    AS_IF([test "x$with_duneweb" != "xno"],[
      # parameter is set. Check it
      AC_MSG_CHECKING([whether passed Dune-Web ($with_duneweb) directory appears correct])
      WEBTESTFILE="$with_duneweb/layout/default.wml"
      AS_IF([test -d "$with_duneweb" -a -e "$WEBTESTFILE"],[
         AC_MSG_RESULT([ok])
         # normalize path
         with_duneweb=`(cd $with_duneweb && pwd)` ;
      ],[
         AS_IF([test -d "$with_duneweb"],[
           AC_MSG_ERROR([$WEBTESTFILE not found in Dune-web dir $with_duneweb!])
         ],[
           AC_MSG_ERROR([Dune-Web directory $with_duneweb not found!])
         ])
         with_duneweb=no
      ])
    ])
     DUNE_ADD_SUMMARY_ENTRY([dune web],[$with_duneweb])
  ],[
    with_duneweb=no
  ])
  AC_SUBST(DUNEWEBDIR, $with_duneweb)
  AM_CONDITIONAL(DUNEWEB, [test "x$with_duneweb" != "xno"])
])
