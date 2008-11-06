AC_DEFUN([DUNE_COMMON_CHECKS],
[
  AC_REQUIRE([AC_PROG_CPP])
  AC_REQUIRE([AC_PROG_CXXCPP])
  AC_REQUIRE([AC_PROG_INSTALL])
  AC_REQUIRE([AC_PROG_LN_S])
  AC_REQUIRE([AC_PROG_MAKE_SET])
  AC_REQUIRE([AC_PROG_RANLIB])
  AC_REQUIRE([AC_PROG_LIBTOOL])

  AC_REQUIRE([DUNE_CHECK_COMPILER])
  AC_REQUIRE([GXX0X])
  AC_REQUIRE([STATIC_ASSERT_CHECK])
  AC_REQUIRE([NULLPTR_CHECK])
  AC_REQUIRE([DUNE_LINKCXX])
  AC_REQUIRE([DUNE_CHECKDEPRECATED])
  AC_REQUIRE([DUNE_SET_MINIMAL_DEBUG_LEVEL])
  AC_REQUIRE([DUNE_PATH_XDR])
  AC_REQUIRE([DUNE_MPI])
  AC_REQUIRE([DUNE_EXPRTMPL])
  AC_REQUIRE([DUNE_DEV_MODE])

  dnl check for programs
  AC_REQUIRE([AC_PROG_CC])
  # add -Wall if the compiler is gcc
  if test "$ac_test_CFLAGS" != set && \
    test "$GCC" = yes; then
    CFLAGS="$CFLAGS -Wall"
  fi
  # add -Wall if the compiler is g++
  AC_REQUIRE([AC_PROG_CXX])
  if test "$ac_test_CXXFLAGS" != set && \
    test "$GXX" = yes; then
    CXXFLAGS="$CXXFLAGS -Wall"
  fi

  dnl checks for header files.
  AC_REQUIRE([AC_HEADER_STDC])
  AC_LANG_PUSH([C++])
  # Allow disabling tr1 headers. E.g. needed for broken xlc on Blue Gene
  AC_ARG_ENABLE(tr1-headers, 
	AC_HELP_STRING([--disable-tr1-headers], 
	[Prevents checking for  tr1 headers like tuple, array, etc. 
	This might be needed for broken compilers like xlc on Blue Gene]), 
	[], [enable_tr1_headers=yes])

  if test "x$enable_tr1_headers" = "xyes" ; then
    tr1_headers="type_traits tr1/type_traits array tr1/array tuple tr1/tuple"
  fi
  AC_CHECK_HEADERS([malloc.h string.h $tr1_headers])
  AC_LANG_POP([C++])

  dnl checks for typedefs, structures, and compiler characteristics.
  #  doesn't work, but we don't need it currently
  #  AC_REQUIRE([AC_HEADER_STDBOOL])
  AC_REQUIRE([AC_C_CONST])
  AC_REQUIRE([AC_C_INLINE])
  AC_REQUIRE([AC_TYPE_SIZE_T])
  AC_REQUIRE([AC_STRUCT_TM])

  dnl check for library functions
  AC_REQUIRE([AC_FUNC_MALLOC])
  #  doesn't work, but we don't need it currently
  #  AC_REQUIRE([AC_FUNC_REALLOC])

  AC_LANG_PUSH([C++])
  AC_CHECK_LIB([m], [pow])
  AC_CHECK_FUNCS([sqrt strchr])
  AC_LANG_POP([C++])

  # check for auxiliary tools so that it's not fatal if they're missing
  AC_CHECK_PROGS([DOXYGEN], [doxygen], [true])
  AC_CHECK_PROGS([TEX], [latex], [true])
  AC_CHECK_PROGS([BIBTEX], [bibtex], [true])
  AC_CHECK_PROGS([DVIPDF], [dvipdf], [true])
  AC_CHECK_PROGS([DVIPS], [dvips], [true])
  AC_CHECK_PROGS([WML], [wml], [true])
  AM_CONDITIONAL([WML], [test "x$WML" != xtrue])
  AC_CHECK_PROGS([PERL], [perl], [true])
  AC_REQUIRE([DUNE_INKSCAPE])
  AC_CHECK_PROGS([CONVERT], [convert], [true])
])

AC_DEFUN([DUNE_COMMON_CHECK_MODULE],
[
    DUNE_CHECK_MODULES([dune-common], [common/stdstreams.hh],
	[#ifndef DUNE_MINIMAL_DEBUG_LEVEL 
   #define DUNE_MINIMAL_DEBUG_LEVEL 1
   #endif
	Dune::derr.active();])
])
