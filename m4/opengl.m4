# $Id$
# searches for OpenGL-stuff

AC_DEFUN([DUNE_PATH_OPENGL],[
dnl use test from autoconf-archive
  AC_REQUIRE([AX_CHECK_GL])

  # now GL_LIBS and GL_CFLAGS is set, just as we want it :)	

  # set variable for summary
  with_opengl="no"
  if test "$no_gl" != "yes" ; then 
    with_opengl="yes"
  fi
])
