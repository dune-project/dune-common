# $Id$

# this meta-check calls everything needed for Dune to work and all
# possible components. Applications should use this so that
# Dune-updates enable new features automagically

# the entries are more or less copied from an "autoscan"-run in the
# dune-directory

#
# There are two test available:
# 1) DUNE_CHECK_ALL
#    This test is for people writing an application based on dune
# 2) DUNE_CHECK_ALL_M
#    This test is for dune modules.
#    In addition to DUNE_CHECK_ALL it run some additional tests
#    and sets up some things needed for modules (i.e. the 'dune' symlink)

# m4_define([DUNE_GET_MODULE_VAR],[m4_esyscmd([grep ^Version: dune.module | cut -d ':' -f 2 | tr -d '\n '])])

m4_define([DUNE_PARSE_MODULE_FILE],[
  m4_define([DUNE_MOD_VERSION], 
    [m4_esyscmd([grep ^Version: dune.module | cut -d ':' -f 2 | tr -d '\n '])])
  m4_define([DUNE_MOD_NAME], 
    [m4_esyscmd([grep ^Module: dune.module | cut -d ':' -f 2 | tr -d '\n '])])
  m4_define([DUNE_MAINTAINER_NAME], 
    [m4_esyscmd([grep ^Maintainer: dune.module | cut -d ':' -f 2 | tr -d '\n '])])
])

m4_define([DUNE_AC_INIT],[
  DUNE_PARSE_MODULE_FILE
  AC_INIT(DUNE_MOD_NAME, DUNE_MOD_VERSION, DUNE_MAINTAINER_NAME)
  # don't build shared libs per default, this is way better for debugging...
  m4_ifdef([LT_INIT],
    [LT_INIT],
    [AC_DEFUN([LT_OUTPUT])])
  AC_DISABLE_SHARED
])

AC_DEFUN([DUNE_CHECK_DEPENDENCIES], [
  AC_REQUIRE([PKG_PROG_PKG_CONFIG])
  AC_PROG_LIBTOOL
  AC_PROG_CXX
  LT_OUTPUT
  DUNE_PARSE_MODULE_FILE
  [## invoke checks for] DUNE_MOD_NAME
  m4_pushdef([_dune_module], [m4_translit(DUNE_MOD_NAME, [-], [_])])
  m4_pushdef([_DUNE_MODULE], [m4_toupper(_dune_module)])
  m4_ifdef(_DUNE_MODULE[]_CHECKS,_DUNE_MODULE[]_CHECKS)
  m4_popdef([_dune_module])
  m4_popdef([_DUNE_MODULE])
  [## invoke checks] DUNE_MOD_NAME depends on
  DUNE_MODULE_DEPENDENCIES(DUNE_MOD_NAME)
  DUNE_MODULE_SUGGESTIONS(DUNE_MOD_NAME)
])

AC_DEFUN([DUNE_CHECK_ALL],[
  # doxygen and latex take a lot of time...
  AC_REQUIRE([DUNE_DOCUMENTATION])
  AC_REQUIRE([DUNE_WEB])

  dnl check all components
  AC_REQUIRE([DUNE_CHECK_DEPENDENCIES])

  # convenience-variables if every found package should be used
  AC_SUBST(ALL_PKG_LIBS, "$LIBS $DUNE_PKG_LIBS")
  AC_SUBST(ALL_PKG_LDFLAGS, "$LDFLAGS $DUNE_PKG_LDFLAGS")
  AC_SUBST(ALL_PKG_CPPFLAGS, "$CPPFLAGS $DUNE_PKG_CPPFLAGS")

  AC_SUBST(am_dir, $DUNE_COMMON_ROOT/am)
])

AC_DEFUN([DUNE_ADD_SUMMARY_ENTRY],[
  indentlen=17
  txt="$1"
  while test `echo "$txt" | tr -d '\n' | wc -c` -lt $indentlen; do txt="$txt."; done
  txt="$txt: $2"
  [DUNE_SUMMARY="$DUNE_SUMMARY echo '$txt';"]
])

AC_DEFUN([DUNE_SUMMARY_ALL],[
  # show search results

  echo
  echo "Found the following Dune-components: "
  echo
  echo "-----------------------------"
  echo  
  [eval $DUNE_SUMMARY]
  echo
  echo "-----------------------------"
  echo
  echo "See ./configure --help and config.log for reasons why a component wasn't found"
  echo

])

AC_DEFUN([DUNE_CHECK_ALL_M],[
  AC_LANG_PUSH([C++])

  AC_REQUIRE([DUNE_SYMLINK])

  # special settings for check-log
  AC_ARG_WITH(hostid,
    AC_HELP_STRING([--with-hostid=HOST_IDENTIFIER],
                 [host identifier used for automated test runs]))
  if test "x$with_hostid" = "xno" ; then 
    with_hostid="$ac_hostname (`uname -sm`, $COMPILER_NAME)";
  fi
  AC_SUBST(host, $with_hostid)
  AC_ARG_WITH(tag,
    AC_HELP_STRING([--with-tag=TAG],
                 [tag to use for automated test runs]))
  if test "x$with_tag" = "xno" ; then with_tag=foo; fi
    AC_SUBST(tag, $with_tag)
  AC_ARG_WITH(revision,
    AC_HELP_STRING([--with-revision=TAG],
                 [revision to use for automated test runs]))
  if test "x$with_revision" = "xno" ; then with_revision=bar; fi
  AC_SUBST(revision, $with_revision)

  AC_REQUIRE([DUNE_CHECK_ALL])
  AC_REQUIRE([DUNE_DEV_MODE])
  AC_REQUIRE([DUNE_PKG_CONFIG_REQUIRES])

  AC_LANG_POP([C++])
])
