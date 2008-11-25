AC_DEFUN([DUNE_ISTL_CHECKS],
[
  # call IMDX_LIB_METIS directly and not via AC_REQUIRE
  # because AC_REQUIRE support not allow parameters
  IMMDX_LIB_METIS(,[true])
  AC_REQUIRE([DUNE_PATH_PARMETIS])
  AC_REQUIRE([DUNE_PATH_SUPERLU])
  AC_REQUIRE([DUNE_PATH_SUPERLU_DIST])
  AC_REQUIRE([DUNE_PARDISO])
  AC_REQUIRE([__AC_FC_NAME_MANGLING])
  AC_REQUIRE([AC_PROG_F77])
  AC_REQUIRE([ACX_BLAS])
  # add summary entries for tests not maintained by dune
  DUNE_ADD_SUMMARY_ENTRY([METIS],[$with_metis])
  DUNE_ADD_SUMMARY_ENTRY([BLAS],[$acx_blas_ok])
])

AC_DEFUN([DUNE_ISTL_CHECK_MODULE],
[
    DUNE_CHECK_MODULES([dune-istl], [istl/allocator.hh])
])
