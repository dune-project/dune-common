# $Id$
# Process this file with autoconf to produce a configure script.
AC_INIT(Dune, 0.0, dune@hal.iwr.uni-heidelberg.de)
AM_INIT_AUTOMAKE(Dune, 0.0, dune@hal.iwr.uni-heidelberg.de)
AC_CONFIG_SRCDIR([grid/common/grid.hh])
AM_CONFIG_HEADER(common/config.hh)

## extra options

AC_ARG_WITH(dim,
            AC_HELP_STRING([--with-dim=[2|3]],[which dimension to use in Albert/UG-libs]),,
            with_dim=2)
AC_SUBST(LIBDIM, $with_dim)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for the right compiler
if test x$GCC = xyes ; then
  if "$CC" --version | grep '^2\.' 1>/dev/null 2>&1; then
    AC_MSG_ERROR([you are using g++ 2.x, please upgrade to g++ 3.x
		  if you have installed different compilers you can specify one
		  by setting the environment variables CXX und CC.
    ])
  fi
fi

# stuff we need for the dune-headers
# !!! add warnings if something can't be found

# Checks for libraries.
AC_CHECK_LIB([m], [pow])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([malloc.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([pow sqrt])

# write output
AC_CONFIG_FILES([Makefile
	         common/Makefile
                 fem/Makefile
                 fem/localbase/Makefile
                 fem/operator/Makefile
                 grid/Makefile
                 grid/albertgrid/Makefile
                 grid/common/Makefile
                 grid/sgrid/Makefile
                 grid/spgrid/Makefile
                 solver/Makefile
                 solver/common/Makefile
                 solver/pmgsolver/Makefile
		 doc/Makefile
		 m4/Makefile
                 dune.pc])
AC_OUTPUT
