# $Id$

# this meta-check calls everything needed for Dune to work and all
# possible components. Applications should use this so that
# Dune-updates enable new features automagically

# the entries are more or less copied from an "autoscan"-run in the
# dune-directory

AC_DEFUN([DUNE_CHECK_ALL],[
  AC_LANG_PUSH([C++])
dnl check for programs
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([AC_PROG_CPP])
  AC_REQUIRE([DUNE_CHECK_COMPILER])
  AC_REQUIRE([DUNE_CHECKDEPRECATED])
  AC_REQUIRE([AC_PROG_INSTALL])
  AC_REQUIRE([AC_PROG_LN_S])
  AC_REQUIRE([AC_PROG_MAKE_SET])
  AC_REQUIRE([AC_PROG_RANLIB])
  AC_REQUIRE([AC_PROG_LIBTOOL])

dnl checks for header files.
  AC_REQUIRE([AC_HEADER_STDC])
  AC_CHECK_HEADERS([malloc.h string.h])

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

dnl check all components
  AC_REQUIRE([DUNE_PATH_DUNE])
  AC_REQUIRE([DUNE_PATH_XDR])
  AC_REQUIRE([DUNE_PATH_GRAPE])
  AC_REQUIRE([DUNE_PATH_ALBERTA])
  AC_REQUIRE([DUNE_PATH_BLAS])
  AC_REQUIRE([DUNE_PATH_UG])
  AC_REQUIRE([DUNE_PATH_F5])
  AC_REQUIRE([DUNE_PATH_AMIRAMESH])
  AC_REQUIRE([DUNE_MPI])
  AC_REQUIRE([DUNE_PATH_ALU3DGRID])
  AC_REQUIRE([DUNE_GETPAGESIZE])

  if test x$HAVE_DUNE != x1 ; then
    AC_MSG_ERROR([Can't work without the DUNE-library. Maybe you have to supply your DUNE-directory as --with-dune=dir])
  fi

  # convenience-variables if every found package should be used
  AC_SUBST(ALL_PKG_LIBS, "$LIBS $DUNE_PKG_LIBS")
  AC_SUBST(ALL_PKG_LDFLAGS, "$LDFLAGS $DUNE_PKG_LDFLAGS")
  AC_SUBST(ALL_PKG_CPPFLAGS, "$CPPFLAGS $DUNE_PKG_CPPFLAGS")
  AC_LANG_POP([C++])
])

AC_DEFUN([DUNE_SUMMARY_ALL],[
  # show search results

  echo
  echo "Found the following Dune-components: "
  echo
  echo "-----------------------------"
  echo  
  echo "Alberta..........: $with_alberta"
  echo "AmiraMesh........: $with_amiramesh"
  echo "BLAS-lib.........: $with_blas"
  echo "Grape............: $with_grape"
  echo "HDF5.............: $with_hdf5"
  echo "FiberHDF5........: $with_f5"
  echo "MPI..............: $with_mpi"
  echo "OpenGL...........: $with_opengl"
  echo "UG...............: $with_ug"
  echo
  echo "-----------------------------"
  echo
  echo "See ./configure --help and config.log for reasons why a component wasn't found"
  echo

])
