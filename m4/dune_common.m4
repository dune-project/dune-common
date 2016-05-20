AC_DEFUN([DUNE_COMMON_CHECKS],
[
  AC_REQUIRE([PKG_PROG_PKG_CONFIG])

  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PROG_CPP])
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([AC_PROG_CXXCPP])
  AC_REQUIRE([AC_PROG_INSTALL])
  AC_REQUIRE([AC_PROG_LN_S])
  AC_REQUIRE([AC_PROG_MAKE_SET])
  AC_REQUIRE([AC_PROG_RANLIB])
  AC_REQUIRE([AC_PROG_LIBTOOL])
  AC_REQUIRE([AC_PROG_AWK])
  AC_REQUIRE([ACX_LAPACK])
  AC_REQUIRE([AC_FC_WRAPPERS])
  # defined $(MKDIR_P), which is required during install
  AC_REQUIRE([AC_PROG_MKDIR_P])

  AC_REQUIRE([DUNE_CHECK_COMPILER])
  AC_REQUIRE([CXX11])
  AC_REQUIRE([NULLPTR_CHECK])
  AC_REQUIRE([LAMBDA_EXPR_CHECK])
  AC_REQUIRE([ALIGNOF_CHECK])
  AC_REQUIRE([CXX11_CONSTEXPR_CHECK])
  AC_REQUIRE([DUNE_CXX11_NOEXCEPT_SPECIFIER_CHECK])
  AC_REQUIRE([DUNE_CXX11_RANGE_BASED_FOR])
  AC_REQUIRE([DUNE_LINKCXX])
  AC_REQUIRE([DUNE_CHECKDEPRECATED])
  AC_REQUIRE([DUNE_CHECKFINAL])
  AC_REQUIRE([DUNE_CHECKUNUSED])
  AC_REQUIRE([DUNE_CXX11_STD_DECLVAL_CHECK])
  AC_REQUIRE([DUNE_CXX11_IS_INDEXABLE_SUPPORT_CHECK])
  AC_REQUIRE([DUNE_CHECK_CXA_DEMANGLE])
  AC_REQUIRE([DUNE_SET_MINIMAL_DEBUG_LEVEL])
  AC_REQUIRE([DUNE_PATH_XDR])
  AC_REQUIRE([DUNE_MPI])
  AC_REQUIRE([DUNE_SYS_MPROTECT])

  AC_REQUIRE([DUNE_STDTHREAD])
  AS_IF([test "x$DUNE_STDTHREAD_WORKS" = xno], [
    AC_MSG_ERROR([dune-common needs working threading support])
  ])
  DUNE_ADD_MODULE_DEPS([dune-common], [STDTHREAD], [${STDTHREAD_CPPFLAGS}],
                       [${STDTHREAD_LDFLAGS}], [${STDTHREAD_LIBS}])

  dnl check for programs
  AC_REQUIRE([AC_PROG_CC])
  # add -Wall if the compiler is gcc
  AS_IF([test "x$ac_test_CFLAGS" != "xset" -a "x$GCC" = "xyes"],[
    CFLAGS="$CFLAGS -Wall"
  ])
  # add -Wall if the compiler is g++
  AC_REQUIRE([AC_PROG_CXX])
  AS_IF([test "x$ac_test_CXXFLAGS" != "xset" -a "x$GXX" = "xyes"],[
    CXXFLAGS="$CXXFLAGS -Wall"
  ])

  dnl checks for header files.
  AC_REQUIRE([AC_HEADER_STDC])
  AC_LANG_PUSH([C++])

  AC_CHECK_HEADERS([malloc.h string.h])

  AC_LANG_POP([C++])

  dnl checks for typedefs, structures, and compiler characteristics.
  #  doesn't work, but we don't need it currently
  #  AC_REQUIRE([AC_HEADER_STDBOOL])
  AC_REQUIRE([AC_C_CONST])
  AC_REQUIRE([AC_C_INLINE])
  AC_REQUIRE([AC_TYPE_SIZE_T])
  AC_REQUIRE([AC_STRUCT_TM])

  AC_CHECK_ALIGNOF(double)
  AC_CHECK_ALIGNOF(float)
  AC_CHECK_ALIGNOF(size_t)
  AC_CHECK_ALIGNOF(uint64_t)
  AC_CHECK_ALIGNOF(uint32_t)

  dnl check for library functions

  AC_LANG_PUSH([C++])
  AC_CHECK_LIB([m], [pow])
  AC_CHECK_FUNCS([sqrt strchr])
  AC_LANG_POP([C++])

  AC_REQUIRE([DUNE_PATH_TBB])
  AC_REQUIRE([DUNE_PATH_GMP])
  AC_REQUIRE([DUNE_PATH_CUDA])

  AS_CASE(["$tbb_summary "],
    ["yes "*], [],
    [AC_MSG_FAILURE([Could not find a working version of Threading Building Blocks, which is required by EXADUNE dune-common.])]
  )
  DUNE_ADD_MODULE_DEPS([dune-common], [TBB],
                       [${TBB_CPPFLAGS}], [${TBB_LDFLAGS}], [${TBB_LIBS}])
])

AC_DEFUN([DUNE_COMMON_CHECK_MODULE],
[
    DUNE_CHECK_MODULES([dune-common], [common/version.hh],
	[#ifndef DUNE_MINIMAL_DEBUG_LEVEL
   #define DUNE_MINIMAL_DEBUG_LEVEL 1
   #endif
	Dune::derr.active();])
])
