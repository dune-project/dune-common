# $Id$
# searches for XDR Headers 

AC_DEFUN([DUNE_PATH_XDR],[
  AC_REQUIRE([AC_PROG_CC])
  AC_CHECK_HEADERS(rpc/xdr.h)
])
