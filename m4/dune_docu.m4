# searches for documentation 
AC_DEFUN([DUNE_DOCUMENTATION],[

  # if dist has been build without
  # documentation then disable documentation 
  # option by default 
  DOCU_TAG_FILE="no-documentation.tag"
  AS_IF([test -f "doc/$DOCU_TAG_FILE"],[
    HAS_DOCUMENTATION="no"],[
    # check whether option is chosen or not 
    AS_IF([test "x$enable_documentation" != "xno"],[
      HAS_DOCUMENTATION="yes"],[
      HAS_DOCUMENTATION="no"
    ])
  ])
  AC_SUBST(DOCUMENTATION_TAG_FILE, $DOCU_TAG_FILE )
  AC_ARG_ENABLE(documentation,
   AS_HELP_STRING([--disable-documentation],[don\'t generate docs, speeds up the build]))
  AM_CONDITIONAL(BUILD_DOCS, [test "x$HAS_DOCUMENTATION" = "xyes"])
])
