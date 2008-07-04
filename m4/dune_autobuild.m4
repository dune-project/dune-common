#
# Add configure flags needed to create log files for dune-autobuild
#

AC_DEFUN([DUNE_AUTOBUILD_FLAGS],[
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
])