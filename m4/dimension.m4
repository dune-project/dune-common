# $Id$
# asks for problem-dimension and world-dimension to pass on to Albert/UG

AC_DEFUN([DUNE_DIMENSION],[

# default dimension of a problem is 2
AC_ARG_WITH(problem_dim,
            AC_HELP_STRING([--with-problem-dim=2|3],
	      [which dimension of the problem]),,with_problem_dim=2)
AC_SUBST(PROBLEMDIM, $with_problem_dim)

# default dimension of the world coordinates is 2
AC_ARG_WITH(world_dim,
            AC_HELP_STRING([--with-world-dim=2|3],
	      [which dimension of the world coordinates]),,with_world_dim=2)
AC_SUBST(PROBLEMDIM, $with_world_dim)

])