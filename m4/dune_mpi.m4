# $Id$

# wrapper for the autoconf-archive check. Note: compiling MPI-stuff sucks!

# Explanation:
# ============
#

# compilation of MPI-programs is normally done by a
# mpicc/mpiCC-wrapper that adds all options needed. Thus, it may seem
# possible to just replace the compiler call by the wrapper and
# everything works. Unfortunately that's not the case: automake and
# libtool both show strange behaviour.
#
# In detail: replacing the compiler globally via ./configure CXX=mpiCC
# should work (at least I've found reports claiming this) but that is
# not what we want: mainly, it just adds a level of possible errors
# (mpiCC from MPICH does _nothing_ if "mpicc -c dummy.cc" is called!)
# and might introduce nice library-clashes.
#
# The next approach would be to include
#       if MPI
#         CXX = $(MPICXX)
#       endif
# in the Makefile.am where MPI is needed. First, this will change
# compilations of all binaries in this directory and secondly the
# dependency-tracking seems to break: the first compilation worked but
# the second failed with the compiler complaining about mismatching
# flags... There is no 'program_CXX = ...' in automake but even if
# there were it would break as well
#
# Thus, the best solution is to extract the flags needed for
# compilation and linking. Unfortunately, the parameters and behaviour
# of mpicc is not at all consistent over different
# implementations. For MPICH the parameters -compile_info and
# -link_info exist (albeit not being documented in the manpage, only
# in -help), for LAM dummy-calls of compilation and linking together
# with a -showme parameter (which is called -show in MPICH...) have to
# be used. Obviously, we have to identify the type of package... this
# is done via mpiCC-calls for now, I wouldn't be surprised if ths
# breaks often. Bad luck. Blame the MPI folks for this mess. And blame
# them a lot. [Thimo 26.8.2004]

# TODO:
#
# - add --disable-mpi

AC_DEFUN([DUNE_MPI],[
  AC_PREREQ(2.50) dnl for AC_LANG_CASE

  # implicitly sets the HAVE_MPI-define and the MPICXX-substitution
  ACX_MPI()

# somehow variables like $1, $2 seem to disappear after m4... Quote them...
dune_mpi_getflags () {
    # -- call mpiCC, remove compiler name
    # compiler-name is first word in line _if_ it doesn't start with a dash!
    # needed because mpiCC sometimes does not include compiler (newer LAM)

    # the additional brackets keep m4 from interpreting the brackets
    # in the sed-command...
    retval=[`$MPICOMP ${1} ${2} 2>/dev/null | sed -e 's/^[^-][^ ]\+ //'`]
    # remove dummy-parameter (if existing)
    if test ${#} = 2 ; then
      retval=`echo $retval | sed -e "s/${2}//"`
    fi
}

  # get compilation script
  AC_LANG_CASE([C],[
	MPICOMP="$MPICC"
],
[C++],[
	MPICOMP="$MPICXX"
]
)

  # taken from acx_mpi: test succeeded if MPILIBS is not empty
  if test x != x"$MPILIBS" -a x != x"$MPICOMP" ; then
    with_mpi="no"

    AC_MSG_CHECKING([MPI-package])
    # the LAM mpiCC knows a -showme parameter
    dune_mpi_getflags "-showme"
    if test x"$retval" != x ; then
      with_mpi="LAM"

      # try new -showme:xxx function
      dune_mpi_getflags "-showme:compile"
      if test x"$retval" != x ; then
        # seems like LAM >= 7.1 which supports extraction of parameters without
        # dummy files
        AC_MSG_RESULT([LAM >= 7.1])
        MPI_CPPFLAGS="$retval"

        dune_mpi_getflags "-showme:link"
        MPI_LDFLAGS="$retval"
      else
        AC_MSG_RESULT([LAM <= 7.0])
        # use -showme and dummy parameters to extract flags        
        AC_LANG_CASE([C], [MPISOURCE="dummy.c"],
	  [C++], [MPISOURCE="dummy.cc"])

        dune_mpi_getflags "-showme" "-c $MPISOURCE"
        MPI_CPPFLAGS="$retval"

        dune_mpi_getflags "-showme" "dummy.o -o dummy"
        MPI_LDFLAGS="$retval"
      fi
    else
      # the MPICH mpiCC knows a -show parameter
      dune_mpi_getflags "-show"
      if test x"$retval" != x ; then
        with_mpi="MPICH"

        # use special commands to extract options      

        dune_mpi_getflags "-compile_info"
        MPI_CPPFLAGS="$retval"
        # hack in option to disable MPICH-C++-bindings...
        AC_LANG_CASE([C++], [MPI_CPPFLAGS="$MPI_CPPFLAGS -DMPICH_SKIP_MPICXX"])

        dune_mpi_getflags "-link_info"
        MPI_LDFLAGS="$retval"

        AC_MSG_RESULT([MPICH])
      else
        # neither MPICH nor LAM....
        AC_MSG_RESULT([unknown])
      fi
    fi

    # fallback... can't extract flags :( 
    if test x$with_mpi = xno ; then
      AC_MSG_WARN([Could not identify MPI-package! Please send a bugreport and tell us what MPI-package you're using])
    fi
  else
    # ACX_MPI didn't find anything
    with_mpi="no"
  fi

  # set flags
  if test x$with_mpi != xno ; then
    AC_SUBST(MPI_CPPFLAGS, $MPI_CPPFLAGS)
    AC_SUBST(MPI_LDFLAGS, $MPI_LDFLAGS)
  else
    AC_SUBST(MPI_CPPFLAGS, "")
    AC_SUBST(MPI_LDFLAGS, "")
  fi

  AM_CONDITIONAL(MPI, test x$with_mpi != xno)
])
