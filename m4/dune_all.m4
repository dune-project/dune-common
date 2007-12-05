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

AC_DEFUN([DUNE_CHECK_ALL],[
  AC_LANG_PUSH([C++])
dnl check for programs
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([AC_PROG_CPP])
  AC_REQUIRE([AC_PROG_CXXCPP])
  AC_REQUIRE([DUNE_CHECK_COMPILER])
  AC_REQUIRE([DUNE_LINKCXX])
  AC_REQUIRE([DUNE_CHECKDEPRECATED])
  AC_REQUIRE([AC_PROG_INSTALL])
  AC_REQUIRE([AC_PROG_LN_S])
  AC_REQUIRE([AC_PROG_MAKE_SET])
  AC_REQUIRE([AC_PROG_RANLIB])
  AC_REQUIRE([AC_PROG_LIBTOOL])

dnl checks for header files.
  AC_REQUIRE([AC_HEADER_STDC])
  AC_CHECK_HEADERS([malloc.h string.h type_traits tr1/type_traits array tr1/array tuple tr1/tuple])

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
  DUNE_INKSCAPE
  AC_CHECK_PROGS([CONVERT], [convert], [true])

  # doxygen and latex take a lot of time...
  AC_REQUIRE([DUNE_DOCUMENTATION])

  # special variable to include the documentation into the website
  AC_ARG_WITH(duneweb,
    AC_HELP_STRING([--with-duneweb=PATH],[Only needed for website-generation, path to checked out version of dune-web]))

if test x$with_duneweb != x ; then
   # parameter is set. Check it
   AC_MSG_CHECKING([whether passed Dune-Web directory appears correct])
   WEBTESTFILE="$with_duneweb/layout/default.wml"
   if test -d "$with_duneweb" && test -e "$WEBTESTFILE" ; then
      AC_MSG_RESULT([ok])
      # normalize path
      with_duneweb=`(cd $with_duneweb && pwd)` ;
   else
      if test -d "$with_duneweb" ; then
        AC_MSG_ERROR([$WEBTESTFILE not found in Dune-web dir $with_duneweb!])
      else
        AC_MSG_ERROR([Dune-Web directory $with_duneweb not found!])
      fi
   fi
fi
AC_SUBST(DUNEWEBDIR, $with_duneweb)

  dnl check all components
  DUNE_MODULE_DEPENDENCIES($@)
  AC_REQUIRE([DUNE_SET_MINIMAL_DEBUG_LEVEL])
  AC_REQUIRE([DUNE_PATH_XDR])
  AC_REQUIRE([DUNE_GRID_DIMENSION])
  AC_REQUIRE([DUNE_PATH_GRAPE])
  AC_REQUIRE([DUNE_PATH_ALBERTA])
  AC_REQUIRE([DUNE_PATH_UG])
  AC_REQUIRE([DUNE_PATH_AMIRAMESH])
  AC_REQUIRE([DUNE_PATH_PSURFACE])
  AC_REQUIRE([DUNE_MPI])
  # call IMDX_LIB_METIS directly and not via AC_REQUIRE
  # because AC_REQUIRE support not allow parameters
  IMMDX_LIB_METIS(,[true])
  AC_REQUIRE([DUNE_PATH_PARMETIS])
  AC_REQUIRE([DUNE_PATH_SUPERLU])
  AC_REQUIRE([DUNE_PATH_SUPERLU_DIST])
  AC_REQUIRE([__AC_FC_NAME_MANGLING])
  AC_REQUIRE([ACX_BLAS])
  AC_REQUIRE([DUNE_PATH_ALUGRID])
  AC_REQUIRE([DUNE_EXPRTMPL])
  AC_REQUIRE([DUNE_PATH_HDF5])

  # convenience-variables if every found package should be used
  AC_SUBST(ALL_PKG_LIBS, "$LIBS $DUNE_PKG_LIBS")
  AC_SUBST(ALL_PKG_LDFLAGS, "$LDFLAGS $DUNE_PKG_LDFLAGS")
  AC_SUBST(ALL_PKG_CPPFLAGS, "$CPPFLAGS $DUNE_PKG_CPPFLAGS")
  AC_LANG_POP([C++])

  AC_SUBST(am_dir, $DUNE_COMMON_ROOT/am)
])

AC_DEFUN([DUNE_SUMMARY],[
  if test xyes == x$2 || test xno == x$2; then
    echo -n "$1"
    echo -n "$2"
    if test x$3 != x; then echo " ($3)"
    else echo; fi
  fi
])

AC_DEFUN([DUNE_SUMMARY_ALL],[
  # show search results

  echo
  echo "Found the following Dune-components: "
  echo
  echo "-----------------------------"
  echo  
  DUNE_SUMMARY([Dune-common......: ], [$with_dune_common], [$DUNE_COMMON_ROOT])
  DUNE_SUMMARY([Dune-grid........: ], [$with_dune_grid], [$DUNE_GRID_ROOT])
  DUNE_SUMMARY([Dune-istl........: ], [$with_dune_istl], [$DUNE_ISTL_ROOT])
  DUNE_SUMMARY([Dune-disc........: ], [$with_dune_disc], [$DUNE_DISC_ROOT])
  DUNE_SUMMARY([Dune-fem.........: ], [$with_dune_fem], [$DUNE_FEM_ROOT])
  echo "ALBERTA..........: $with_alberta"
  echo "ALUGrid..........: $with_alugrid"
  echo "AmiraMesh........: $with_amiramesh"
  echo "BLAS.............: $acx_blas_ok"
  echo "Grape............: $with_grape"
  echo "HDF5.............: $with_hdf5"
#  echo "FiberHDF5........: $with_f5"
  echo "MPI..............: $with_mpi"
  echo "METIS............: $with_metis"
  echo "ParMETIS.........: $with_parmetis"
  echo "SuperLU..........: $with_superlu"
  echo "SuperLU-DIST.....: $with_superlu_dist"	
  echo "OpenGL...........: $with_opengl"
  echo "UG...............: $with_ug"
  echo
  echo "-----------------------------"
  echo
  echo "See ./configure --help and config.log for reasons why a component wasn't found"
  echo

])

AC_DEFUN([DUNE_CHECK_ALL_M],[
  AC_LANG_PUSH([C++])

  AC_REQUIRE([DUNE_SYMLINK])
  # don't build shared libs per default, this is way better for debugging...
  AC_REQUIRE([AC_DISABLE_SHARED])

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

  DUNE_MODULE_DEPENDENCIES($@)
  AC_REQUIRE([DUNE_CHECK_ALL])
  AC_REQUIRE([DUNE_DEV_MODE])
  AC_REQUIRE([DUNE_PKG_CONFIG_REQUIRES])

  AC_LANG_POP([C++])
])
