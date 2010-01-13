dnl -*- autoconf -*-
# Just like AC_CHECK_LIB, but in allow a library path to be specified.  The
# default ACTION-IF-FOUND is extended to also add that library path to LIBS.
#
# DUNE_CHECK_LIB_EXT(PATH, LIBRARY, FUNCTION, [ACTION-IF-FOUND],
#                    [ACTION-IF-NOT-FOUND], [OTHER-LIBRARIES])
AC_DEFUN([DUNE_CHECK_LIB_EXT],
[
  dune_cle_save_LDFLAGS="$LDFLAGS"
  LDFLAGS="$LDFLAGS -L$1"
  m4_if(m4_eval([$# <= 3]), [1],
    [AC_CHECK_LIB([$2], [$3],
      [
        LIBS="-L$1 -l$2 $LIBS"
        AC_DEFINE([HAVE_LIB]m4_translit([[$2]], [-a-z], [_A-Z]), [1],
                  [Define to 1 if you have the `$2' library (-l$2).])
      ])],
    [AC_CHECK_LIB(m4_shift($@))])
  LDFLAGS="$dune_cle_save_LDFLAGS"
])
