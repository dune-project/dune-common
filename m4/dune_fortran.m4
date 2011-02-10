# a trick to make life for the  user easier.
#
# some libs need the fortran test, while other use the f77 test. This
# requires the user to provide two variables, usually with the same
# parameter. In most cases the F77 compiler will be the same as teh FC
# compiler.
#
# We test for FC and F77. If the user has specified both, we do
# nothing, if the user has specified only one of them, we assume that
# the compiler supports both and copy the value. Usually this should
# be the sane default.
#
AC_DEFUN([DUNE_SYNC_FC_F77],[
  # variant 1
  # FC is set, while F77 is not set
  AS_IF([test -n "$FC" && test -z "$F77"], [
    AC_MSG_NOTICE([Fortran compiler FC (fortran 90) specified, 
      while F77 (fortran 77) wasn't set.
      Trying to use FC as F77.])
    F77="$FC"
  ])
  # variant 2
  # F77 is set, while FC is not set
  AS_IF([test -z "$FC" && test -n "$F77"], [
    AC_MSG_NOTICE([Fortran compiler F77 (fortran 77) specified, 
      while FC (fortran 90) wasn't set.
      Trying to use F77 as FC.])
    FC="$F77"
  ])
])

