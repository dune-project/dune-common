# $Id$
# searches for OpenGL-stuff

AC_DEFUN(DUNE_PATH_OPENGL,
[
  AC_REQUIRE([AC_PROG_CC])
	
  AC_CHECK_HEADERS(GL/gl.h)
  AC_CHECK_LIB(GL, glNewList, [GL_LIBS="-lGL"], [have_gl=no])

  if test x$have_gl != xno ; then 
    AC_SUBST(GL_LIBS, $GL_LIBS)
  fi

])