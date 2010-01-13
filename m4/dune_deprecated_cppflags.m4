dnl -*- autoconf -*-
# Define configure substitution SUBSTITUTION.  Assign value to it but add an
# additional preprocessor define which may be detected by config.h.  Add some
# text to config.h which detects that define and issues the warning in
# message.
#
# DUNE_DEPRECATED_CPPFLAGS(SUBSTITUTION, VALUE, MESSAGE)
AC_DEFUN([DUNE_DEPRECATED_CPPFLAGS],
[
  AH_VERBATIM([DEPRECATED_$1_USED],
[#ifdef DEPRECATED_$1_USED
#warning $3
#endif])
  AC_SUBST([$1], [$2" -D DEPRECATED_$1_USED"])
])
