## -*- autoconf -*-
# $Id$
# searches for OpenGL-stuff

# DUNE_PATH_OPENGL()
#
# Call AX_CHECK_OPENGL, but also add the discovered flags too the ALL_PKG_*
# family and add a summary entry.  Also make sure that pthreads are supported.
AC_DEFUN([DUNE_PATH_OPENGL],[
dnl use test from autoconf-archive
  AC_REQUIRE([AX_CHECK_GL])

  # GL_LIBS and GL_CFLAGS already set correctly, they implicitly
  # contain pthread support!

  # variable for summary
  with_opengl="no"
  if test "$no_gl" != "yes" ; then 
    with_opengl="yes (add GL_LIBS to LDADD manually, etc.)"

    # need pthreads-stuff in linker commandline as well
    AC_SUBST(GL_LDFLAGS, $PTHREAD_CFLAGS)
    # GL_CFLAGS contain stuff like -pthread, which according to the gcc
    # documentation is a flag for both the preprocessor and the linker.  So we
    # add it to the preprocessor flags here.
    AC_SUBST(GL_LIBS, $GL_LIBS)
    AC_SUBST(GL_LDFLAGS, $GL_LDFLAGS)
    AC_SUBST(GL_CPPFLAGS, $GL_CFLAGS)
  fi

  DUNE_ADD_SUMMARY_ENTRY([OpenGL],[$with_opengl])
])
