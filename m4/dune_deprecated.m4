# $Id$

# Check for the right way to create the deprecated warning

AC_DEFUN([DUNE_CHECKDEPRECATED],[
	AC_MSG_CHECKING([for __attribute__((deprecated))])
	AC_TRY_COMPILE([int foo() __attribute__((deprecated));
                    int foo() {};],[],
				   [DUNE_DEPRECATED="__attribute__((deprecated))"
                    AC_MSG_RESULT(yes)],
				   [DUNE_DEPRECATED=""
                    AC_MSG_RESULT(no)])

    AC_DEFINE_UNQUOTED(DUNE_DEPRECATED, $DUNE_DEPRECATED,
                      [how to create a deprecated warning])
])
