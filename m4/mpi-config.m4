AC_DEFUN([MPI_CONFIG_HELPER],[

mpi_trybuild () {
  $MPICC ${1} > /dev/null 2>&1 || return 1
  return 0
}

mpi_trybuild_run () {
  mpi_trybuild "-o conftest ${1}" || return 1
  ./conftest 2>&1 || return 1
  return 0
}

mpi_preprocess () {
  $MPICC -E -c ${1} 2> /dev/null
}

mpi_getflags () {
  # -- call mpiCC, remove compiler name
  # compiler-name is first word in line _if_ it doesn't start with a dash!
  # needed because mpiCC sometimes does not include compiler (newer LAM)

  # the additional brackets keep m4 from interpreting the brackets
  # in the sed-command...
  retval=`$MPICC ${1} ${2} 2>/dev/null | head -1`
  # remove compiler name
  retval=`echo $retval | sed -e 's/^[[^-]][[^ ]][[^ ]]* //'`
  # remove dummy-parameter (if existing)
  retval=`echo $retval | sed -e "s/ ${1} / /"`
  if test ${#} = 2 ; then
    retval=`echo $retval | sed -e "s/ ${2} / /"`
  fi
  # remove optimization, warning, etc paramters
  retval=`echo " $retval " | sed -e 's/ -g / /g' -e 's/ -W[[a-z0-9]]\+ / /g' -e 's/ -O[[0-9]]\+ / /g'`
  # strip leading and trailing spaces
  retval=`echo "$retval" | sed 's/^ *//g;s/ *$//g'`
}

# removes regexp $2 from string $1
mpi_remove () {
  retval=`echo ${1} | sed -e "s/ ${2} / /"`
  # string to remove might be on the beginning of the line
  retval=`echo ${retval} | sed -e "s/^${2} / /"`
}

test_lam () {
  AC_MSG_CHECKING([for lam])
  cat >conftest.c <<_EOF
#include <mpi.h>
#include <stdio.h>

int main() {
  printf ("%i%i\n", LAM_MAJOR_VERSION, LAM_MINOR_VERSION);
  return 0;
 }
_EOF

  if mpi_trybuild "-c conftest.c"; then
    # try new -showme:xxx function
    mpi_getflags "-showme:compile"
    if test x"$retval" != x ; then
      # seems like LAM >= 7.1 which supports extraction of parameters without
      # dummy files
      dune_MPI_VERSION="LAM >= 7.1"
      if test x"$DUNEMPICPPFLAGS" = x; then
        DUNEMPICPPFLAGS="$retval"
      fi
      if test x"$DUNEMPILIBS" = x; then
        mpi_getflags "-showme:link"
        DUNEMPILIBS="$retval"
      fi
    else
      dune_MPI_VERSION="LAM < 7.1"
      # use -showme and dummy parameters to extract flags        
      if test x"$DUNEMPICPPFLAGS" = x; then
        mpi_getflags "-showme" "-c $MPISOURCE"
        DUNEMPICPPFLAGS="$retval"
      fi
      if test x"$DUNEMPILIBS" = x; then
        mpi_getflags "-showme" "dummy.o -o dummy"
        DUNEMPILIBS="$retval"
      fi
    fi
    # hack in option to disable LAM-C++-bindings...
    # we fake to have mpicxx.h read already
    MPI_NOCXXFLAGS="-DMPIPP_H"
    AC_MSG_RESULT([yes])
    rm -f conftest*
    return 0    
  fi

  rm -f conftest*
  AC_MSG_RESULT([no])
  return 1
}

mpi_getmpichflags() {
  if test x"$DUNEMPICPPFLAGS" = x; then
    # use special commands to extract options      
    mpi_getflags "-compile_info"
    DUNEMPICPPFLAGS="$retval"
    # remove implicitly set -c
    mpi_remove "$DUNEMPICPPFLAGS" '-c'
    DUNEMPICPPFLAGS="$retval"
  fi
    
  if test x"$DUNEMPILIBS" = x; then
    # get linker options
    mpi_getflags "-link_info"
    DUNEMPILIBS="$retval"
    # strip -o option
    mpi_remove "$DUNEMPILIBS" "-o"
    DUNEMPILIBS="$retval"
    #strip DUNEMPICPPFLAGS (which are included for mpich2 on jugene)
    enc=`echo "$DUNEMPICPPFLAGS" | sed -e 's/\\//\\\\\\//g'`
    DUNEMPILIBS=`echo "$retval" | sed -e "s/$enc / /"`
  fi

  # hack in option to disable MPICH-C++-bindings...
  MPI_NOCXXFLAGS="-DMPICH_SKIP_MPICXX"
}

mpi_getmpich2flags() {
  if test x"$DUNEMPICPPFLAGS" = x; then
    # use special commands to extract options      
    mpi_getflags "-show" "-c"
    DUNEMPICPPFLAGS="$retval"
    # remove implicitly set -c
    mpi_remove "$DUNEMPICPPFLAGS" '-c'
    DUNEMPICPPFLAGS="$retval"
  fi
    
  if test x"$DUNEMPILIBS" = x; then
    # get linker options
    mpi_getflags "-show" "-o"
    DUNEMPILIBS="$retval"
    # strip -o option
    mpi_remove "$DUNEMPILIBS" "-o"
    DUNEMPILIBS="$retval"
    #strip DUNEMPICPPFLAGS (which are included for mpich2 on jugene)
    enc=`echo "$DUNEMPICPPFLAGS" | sed -e 's/\\//\\\\\\//g'`
    DUNEMPILIBS=`echo "$retval" | sed -e "s/$enc / /"`
  fi

  # hack in option to disable MPICH-C++-bindings...
  MPI_NOCXXFLAGS="-DMPICH_SKIP_MPICXX"
}

test_mpich () {
  AC_MSG_CHECKING([for mpich])
  cat >conftest.c <<_EOF
#include <mpi.h>

int main() { return 0; }
_EOF

  if (mpi_preprocess conftest.c \
      | grep -q MPICHX_PARALLELSOCKETS_PARAMETERS); then
    dune_MPI_VERSION="MPICH"
    mpi_getmpichflags

    AC_MSG_RESULT([yes])
    rm -f conftest*
    return 0    
  fi

  rm -f conftest*
  AC_MSG_RESULT([no])
  return 1
}

test_mpich2 () {
  AC_MSG_CHECKING([for mpich2])
  cat >conftest.c <<_EOF
#include <mpi.h>
#include <stdio.h>
int main() { printf ("%s\n", MPICH2_VERSION); return 0; }
_EOF

  if mpi_trybuild "-c conftest.c"; then
    dune_MPI_VERSION="MPICH2"
    mpi_getmpich2flags

    AC_MSG_RESULT([yes])
    rm -f conftest*
    return 0
  fi

  rm -f conftest*
  AC_MSG_RESULT([no])
  return 1
}

test_mpich3 () {
  AC_MSG_CHECKING([for mpich 3.x])
  cat >conftest.c <<_EOF
#include <mpi.h>
#include <stdio.h>
int main() { printf ("%s\n", MPICH_VERSION); return 0; }
_EOF

  if (mpi_trybuild_run "conftest.c" | grep -q "^3\.") ; then
    dune_MPI_VERSION="MPICH2"
    mpi_getmpich2flags

    AC_MSG_RESULT([yes])
    rm -f conftest*
    return 0
  fi

  rm -f conftest*
  AC_MSG_RESULT([no])
  return 1
}

test_openmpi () {
  AC_MSG_CHECKING([for OpenMPI])
  cat >conftest.c <<_EOF
#include <mpi.h>

int main() { return 0; }
_EOF

  if (mpi_preprocess conftest.c | grep -q ompi_communicator_t); then
    dune_MPI_VERSION="OpenMPI"

    if test x"$DUNEMPICPPFLAGS" = x; then
      mpi_getflags "-showme:compile"
      DUNEMPICPPFLAGS="$retval"
    fi
    if test x"$DUNEMPILIBS" = x; then
      mpi_getflags "-showme:link"
      DUNEMPILIBS="$retval"
    fi
    MPI_NOCXXFLAGS="-DMPIPP_H"

    AC_MSG_RESULT([yes])
    rm -f conftest*
    return 0    
  fi

  rm -f conftest*
  AC_MSG_RESULT([no])
  return 1
}

test_mvapich() {
  AC_MSG_CHECKING([for MVAPICH])

  mpi_getflags "-v" "-c dummy.c"
  if (echo $dune_MPI_VERSION | grep ^MVAPICH>/dev/null);then
      get_mpichflags

      AC_MSG_RESULT([yes])
      return 0 
  fi

  AC_MSG_RESULT([no])
  return 1
}

test_mvapich2() {
  AC_MSG_CHECKING([for MVAPICH2])
  cat >conftest.c <<_EOF
#define _OSU_MVAPICH_
#include <mpi.h>
/* MVAPICH2_VERSION is only defined for MVAPICH2 1.4+
 * MVAPICH_VERSION is only defined for MVAPICH2 1.2.*
 * We can thus fall back to MVAPICH_VERSION if MVAPICH2_VERSION
 * is not defined.
 */
#ifndef MVAPICH2_VERSION
#define MVAPICH2_VERSION MVAPICH_VERSION
#endif
#include <stdio.h>
int main() { printf("%s\n",MVAPICH2_VERSION); return 0; }
_EOF

  if mpi_trybuild "-c conftest.c"; then
    dune_MPI_VERSION="MVAPICH2"
    mpi_getmpich2flags

    AC_MSG_RESULT([yes])
    rm -f conftest*
    return 0 
  fi

  rm -f conftest*
  AC_MSG_RESULT([no])
  return 1
}

test_ibmmpi() {
  AC_MSG_CHECKING([for IBM MPI])
  if $MPICC -v -c conftest.c > /dev/null 2>&1; then
    mpi_getflags "-v" "-c dummy.c"
    if (echo $retval | grep '^xl[[cC]]'); then
      dune_MPI_VERSION="IBM MPI"

      if test x"$DUNEMPICPPFLAGS" = x; then
        DUNEMPICPPFLAGS="$retval"
      fi
  
      if test x"$DUNEMPILIBS" = x; then
        mpi_getflags "-v" "dummy.o -o dummy"
        DUNEMPILIBS="$retval"
      fi

      AC_MSG_RESULT([yes])
      rm -f conftest*
      return 0  
    fi
  fi
  
  AC_MSG_RESULT([no])
  return 1
}

test_intelmpi() {
  AC_MSG_CHECKING([for Intel MPI])
  mpi_getflags "-v"
  if (echo $retval | grep 'Intel(R) MPI Library'); then
    dune_MPI_VERSION="Intel MPI"
    mpi_getmpich2flags

    AC_MSG_RESULT([yes])
    return 0
  fi

  AC_MSG_RESULT([no])
  return 1
}

get_mpiparameters() {
  AC_MSG_NOTICE([Trying to identify the version of MPI compiler $MPICC])

  if test x"$dune_MPI_VERSION" != x; then
    return
  fi 

  test_lam && return
  test_mpich && return
  test_openmpi && return
  test_mvapich && return
  test_mvapich2 && return
  test_mpich2 && return
  test_mpich3 && return
  test_ibmmpi && return
  test_intelmpi && return
   
  dune_MPI_VERSION="unknown"
  AC_MSG_ERROR([Could not identify MPI-package! Please send a bugreport and tell us what MPI-package you're using.])
}
])

AC_DEFUN([MPI_CONFIG],[
  AC_REQUIRE([MPI_CONFIG_HELPER])  
  get_mpiparameters;
])
