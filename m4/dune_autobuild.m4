#
# Add configure flags needed to create log files for dune-autobuild
#

AC_DEFUN([DUNE_AUTOBUILD_FLAGS],[
  # special settings for check-log
  AC_ARG_WITH([hostid],
    AS_HELP_STRING([--with-hostid=HOST_IDENTIFIER],
                 [host identifier used for automated test runs]))
  AS_IF([test "x$with_hostid" = "xno"],[
    with_hostid="$ac_hostname (`uname -sm`, $COMPILER_NAME)";
  ])

  AC_SUBST([hostid], [$with_hostid])
  AC_ARG_WITH([tag],
    AS_HELP_STRING([--with-tag=TAG],
                 [tag to use for automated test runs]))
  AS_IF([test "x$with_tag" = "xno"],[
    with_tag=foo
  ])
    AC_SUBST([tag], [$with_tag])
  AC_ARG_WITH([revision],
    AS_HELP_STRING([--with-revision=TAG],
                 [revision to use for automated test runs]))
  AS_IF([test "x$with_revision" = "xno"],[
    with_revision=bar])
  AC_SUBST([revision], [$with_revision])
])