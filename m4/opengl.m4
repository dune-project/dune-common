# $Id$
# searches for OpenGL-stuff

AC_DEFUN([DUNE_PATH_OPENGL],[
dnl use test from autoconf-archive
  AC_REQUIRE([AX_CHECK_GL])

  # GL_LIBS and GL_CFLAGS already set correctly, they implicitly
  # contain pthread support!

  # variable for summary
  with_opengl="no"
  if test "$no_gl" != "yes" ; then 
    with_opengl="yes"

    # need pthreads-stuff in linker commandline as well
    AC_SUBST(GL_LDFLAGS, $PTHREAD_CFLAGS)
  fi

  DUNE_ADD_SUMMARY_ENTRY([OpenGL],[$with_opengl])
])
