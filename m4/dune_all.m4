# $Id$

# this meta-check calls everything needed for Dune to work and all
# possible components. Applications should use this so that
# Dune-updates enable new features automagically

# the entries are more or less copied from an "autoscan"-run in the
# dune-directory

AC_DEFUN([DUNE_CHECK_ALL],[
dnl check for programs
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([AC_PROG_CPP])
  AC_REQUIRE([AC_PROG_INSTALL])
  AC_REQUIRE([AC_PROG_LN_S])
  AC_REQUIRE([AC_PROG_MAKE_SET])
  AC_REQUIRE([AC_PROG_RANLIB])
  AC_REQUIRE([AC_PROG_LIBTOOL])

dnl checks for header files.
  AC_REQUIRE([AC_HEADER_STDC])
  AC_CHECK_HEADERS([malloc.h string.h])

dnl checks for typedefs, structures, and compiler characteristics.
  AC_REQUIRE([AC_HEADER_STDBOOL])
  AC_REQUIRE([AC_C_CONST])
  AC_REQUIRE([AC_C_INLINE])
  AC_REQUIRE([AC_TYPE_SIZE_T])
  AC_REQUIRE([AC_STRUCT_TM])

dnl check for library functions
  AC_REQUIRE([AC_FUNC_MALLOC])
  AC_REQUIRE([AC_FUNC_REALLOC])

  AC_LANG_PUSH([C++])
  AC_CHECK_LIB([m], [pow])
  AC_CHECK_FUNCS([sqrt strchr])
  AC_LANG_POP([C++])

dnl check all components
  AC_REQUIRE([DUNE_PATH_DUNE])
  AC_REQUIRE([DUNE_PATH_XDR])
  AC_REQUIRE([DUNE_PATH_GRAPE])
  AC_REQUIRE([DUNE_PATH_ALBERT])
  AC_REQUIRE([DUNE_PATH_BLAS])
  AC_REQUIRE([DUNE_PATH_UG])
  AC_REQUIRE([DUNE_PATH_F5])
  AC_REQUIRE([DUNE_PATH_AMIRAMESH])

  if test x$HAVE_DUNE != x1 ; then
    AC_MSG_ERROR([Can't work without the DUNE-library. Maybe you have to supply your DUNE-directory as --with-dune=dir])
  fi

])