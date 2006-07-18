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

# Sometimes ACX_MPI will not be able to find the correct MPI compiler,
# or sometimes you might have several MPI installations. Specify the
# MPICC variable to enforce a certain MPI compiler.

# In order to diasble the usage of MPI (and make Dune pure
# sequentiell) you can supply the option
#   --disable-parallel
# [Christian 9.7.2006]

AC_DEFUN([DUNE_MPI],[
  AC_PREREQ(2.50) dnl for AC_LANG_CASE

  AC_LANG_PUSH([C])

  # enable/disable parallel features
  AC_ARG_ENABLE(parallel,
    AC_HELP_STRING([--disable-parallel],
      [Try to use the parallel feature of Dune. This option requires MPI to be present. 
       configure will try to determin your MPI automatically,
       you can overwrite this setting by specifying the MPICC veriable]),
    [with_parallel=$enableval],
    [with_parallel=yes]
  )

  # disable runtest if we have a queuing system
  AC_ARG_ENABLE(mpiruntest,
    AC_HELP_STRING([--disable-mpiruntest],
      [Don't try to run a MPI program during configure. (This is need if you depend on a queuing system)]),
    [mpiruntest=${enableval}],
    [mpiruntest=yes]
  )

  with_mpi="no"

  # //
  # helper shell functions

    # somehow variables like $1, $2 seem to disappear after m4... Quote them...
    dune_mpi_getflags () {
      # -- call mpiCC, remove compiler name
      # compiler-name is first word in line _if_ it doesn't start with a dash!
      # needed because mpiCC sometimes does not include compiler (newer LAM)

      # the additional brackets keep m4 from interpreting the brackets
      # in the sed-command...
      retval=[`$MPICOMP ${1} ${2} 2>/dev/null | head -1 | sed -e 's/^[^-][^ ]\+ //'`]
      # remove dummy-parameter (if existing)
      if test ${#} = 2 ; then
        retval=`echo $retval | sed -e "s/${2}//"`
      fi
    }
    # removes regexp $2 from string $1
    dune_mpi_remove () {
      retval=`echo ${1} | sed -e "s/${2}//"`
    }
  # //

  # 1) no paramter : ''
  #    => use ACX_MPI to find the mpi Compiler
  # 2) --with-mpi=/opt/special-mpi/bin/mpicc : '/opt/special-mpi/bin/mpicc'
  #    => use /opt/special-mpi/bin/mpicc as MPI compiler
  # 3) --without-mpi : 'no'
  #    => disable MPI

  ## do nothing if --disable-parallel is used
  if test x$with_parallel == xyes ; then
  
    # is the mpi compilation script already specified?
    AC_LANG_CASE([C],[
        MPICOMP="$MPICC"
      ],
      [C++],[
        MPICOMP="$MPICXX"
      ])
	AC_MSG_NOTICE([user specific mpi compiler would be ... $MPICC])	
    # implicitly sets the HAVE_MPI-define and the MPICXX-substitution
    if test x == x"$MPICOMP"; then
        ACX_MPI()
    else
        AC_MSG_NOTICE([using user specific mpi compiler... $MPICC])
	fi
    ACX_MPI()
    # remove HAVE_MPI from confdefs.h
    cp confdefs.h confdefs.h.tmp
    grep -v "^#define HAVE_MPI " confdefs.h.tmp > confdefs.h
    rm -f confdefs.h.tmp
    # get compilation script
    AC_LANG_CASE([C],[
        MPICOMP="$MPICC"
        dune_mpi_isgnu="$GCC"
      ],
      [C++],[
        MPICOMP="$MPICXX"
        dune_mpi_isgnu="$GXX"
      ])
  
    # taken from acx_mpi: test succeeded if MPILIBS is not empty
    if test x != x"$MPICOMP" ; then

      AC_MSG_CHECKING([for a known MPI package])
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
        # hack in option to disable MPICH-C++-bindings...
        # we fake to have mpicxx.h read already
        MPI_CPPFLAGS="$MPI_CPPFLAGS -DMPIPP_H"
      else
        # the MPICH mpiCC knows a -show parameter
        dune_mpi_getflags "-show"
        if test x"$retval" != x ; then
          with_mpi="MPICH"
          # use special commands to extract options      
          dune_mpi_getflags "-compile_info"
          MPI_CPPFLAGS="$retval"
          # hack in option to disable MPICH-C++-bindings...
          MPI_CPPFLAGS="$MPI_CPPFLAGS -DMPICH_SKIP_MPICXX"
          # remove implicitly set -c
          dune_mpi_remove "$MPI_CPPFLAGS" '-c'
          MPI_CPPFLAGS="$retval"
          dune_mpi_getflags "-link_info"
          MPI_LDFLAGS="$retval"
          AC_MSG_RESULT([MPICH])
        else  
          # check exitcode of -v
          if $MPICOMP -v -c $MPISOURCE > /dev/null 2>&1 ; then
            AC_MSG_RESULT([IBM MPI])
            with_mpi="IBM MPI"
  
            dune_mpi_getflags "-v" "-c dummy.c"
            # mpCC passes on it's own parameter...
            retval=`echo $retval | sed -e "s/-v//"`
            # remove compiler name (double bracket to quote for m4)
            retval=`echo $retval | sed -e 's/^xl[[cC]] //'`
            # remove stuff we passed
            retval=`echo $retval | sed -e "s/-c dummy.c//"`
  
            # mpCC assumes xlc is used...
            if test x$dune_mpi_isgnu = xyes ; then
              # change commandline if GNU compiler is used
              retval=`echo $retval | sed -e 's/\(-b[[^ ]]*\)/-Xlinker \1/g'`
            fi
            MPI_CPPFLAGS="$retval"
  
            dune_mpi_getflags "-v" "dummy.o -o dummy"
            # mpCC passes on it's own parameter...
            retval=`echo $retval | sed -e "s/-v//"`
            # remove compiler name
            retval=`echo $retval | sed -e 's/^xl[[cC]] //'`
            # remove stuff we passed
            retval=`echo $retval | sed -e "s/dummy.o -o dummy//"`

            if test x$dune_mpi_isgnu = xyes ; then
              # change commandline if GNU compiler is used
              retval=`echo $retval | sed -e 's/\(-b[[^ ]]*\)/-Xlinker \1/g'`
            fi

            MPI_LDFLAGS="$retval"
          else
            # don't know MPI....
            AC_MSG_RESULT([unknown])
          fi
        fi
      fi

      # fallback... can't extract flags :( 
      if test x"$with_mpi" = xno ; then
        AC_MSG_WARN([Could not identify MPI-package! Please send a bugreport and tell us what MPI-package you're using])
      fi
    else
      # ACX_MPI didn't find anything
      with_mpi="no"
    fi
  fi # end of MPI identification

  # if an MPI implementation was found..
  if test x"$with_mpi" != xno ; then
    ### do a sanity check: can we compile and link a trivial MPI program?

    # store old values
    ac_save_LIBS="$LIBS"
    ac_save_CPPFLAGS="$CPPFLAGS"
    
    # looks weird but as the -l... are contained in the MPI_LDFLAGS these
    # parameters have to be last on the commandline: with LIBS this is true
    LIBS="$MPI_LDFLAGS"
    CPPFLAGS="$CPPFLAGS $MPI_CPPFLAGS"

    if test "x$mpiruntest" != "xyes" ; then
      AC_MSG_WARN([Diabled test whether compiling/running with $with_mpi works.])    
    else
      if test x"$with_mpi" = xLAM ; then
        AC_MSG_NOTICE([Starting "lamboot" for checking...])
        lamboot -H
        sleep 2s
        AC_MSG_NOTICE(["lamboot" started...])
      fi

      # try to create c++ program
	  AC_LANG_PUSH([C++])
      AC_MSG_CHECKING([whether compiling/running with $with_mpi works])
      AC_RUN_IFELSE(
        AC_LANG_SOURCE(
          [ #include <mpi.h>
            int main (int argc, char** argv) { 
            MPI_Init(&argc, &argv); 
            MPI_Finalize(); }]),
          [ AC_MSG_RESULT([yes]) ],
          [ AC_MSG_RESULT([no])
            AC_MSG_WARN([could not compile or run MPI testprogram, deactivating MPI! See config.log for details])
            with_mpi=no]
      )
	  AC_LANG_POP

      if test x"$with_mpi" = xLAM ; then
        AC_MSG_NOTICE([Stopping LAM via "lamhalt"...])
        lamhalt -H; sleep 2s
        AC_MSG_NOTICE(["lamboot" stopped...])
      fi
    fi

    # restore variables
    LIBS="$ac_save_LIBS"
    CPPFLAGS="$ac_save_CPPFLAGS"
  fi
    
  # set flags
  if test x"$with_mpi" != xno ; then
    AC_SUBST(MPI_CPPFLAGS, $MPI_CPPFLAGS)
    AC_SUBST(MPI_LDFLAGS, $MPI_LDFLAGS)
    AC_DEFINE(HAVE_MPI,1,[Define if you have the MPI library.])
  else
    AC_SUBST(MPI_CPPFLAGS, "")
    AC_SUBST(MPI_LDFLAGS, "")
  fi

  AM_CONDITIONAL(MPI, test x"$with_mpi" != xno)

  AC_LANG_POP
])
