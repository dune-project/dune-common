# searches for documentation 
AC_DEFUN([DUNE_DOCUMENTATION],[

  # if dist has been build without
  # documentation then disable documentation 
  # option by default 
  DOCU_TAG_FILE="no-documentation.tag"
  if test -f doc/$DOCU_TAG_FILE ; then 
    HAS_DOCUMENTATION="no"
  else 
    # check whether option is chosen or not 
    if test x$enable_documentation != xno; then 
      HAS_DOCUMENTATION="yes"
    else 
      HAS_DOCUMENTATION="no"
    fi
  fi
  AC_SUBST(DOCUMENTATION_TAG_FILE, $DOCU_TAG_FILE )
  AC_ARG_ENABLE(documentation,
   AC_HELP_STRING([--disable-documentation],[don't generate docs, speeds up the build]))
  AM_CONDITIONAL(BUILD_DOCS, test x$HAS_DOCUMENTATION = xyes)
])
