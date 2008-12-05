AC_DEFUN([MPI_CONFIG_HELPER],[

mpi_trybuild () {
  $MPICC ${1} > /dev/null 2>&1 || return 1
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
      MPI_VERSION="LAM >= 7.1"
      MPI_CPPFLAGS="$retval"
      mpi_getflags "-showme:link"
      MPI_LDFLAGS="$retval"
    else
      MPI_VERSION="LAM < 7.1"
      # use -showme and dummy parameters to extract flags        
      mpi_getflags "-showme" "-c $MPISOURCE"
      MPI_CPPFLAGS="$retval"
      mpi_getflags "-showme" "dummy.o -o dummy"
      MPI_LDFLAGS="$retval"
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
    # use special commands to extract options      
    mpi_getflags "-compile_info"
    MPI_CPPFLAGS="$retval"
    # remove implicitly set -c
    mpi_remove "$MPI_CPPFLAGS" '-c'
    MPI_CPPFLAGS="$retval"
    
    # get linker options
    mpi_getflags "-link_info"
    MPI_LDFLAGS="$retval"
    # strip -o option
    mpi_remove "$MPI_LDFLAGS" "-o"
    MPI_LDFLAGS="$retval"
    #strip MPI_CPPFLAGS (which are included for mpich2 on jugene)
    enc=`echo "$MPI_CPPFLAGS" | sed -e 's/\\//\\\\\\//g'`
    MPI_LDFLAGS=`echo "$retval" | sed -e "s/$enc / /"`


    # hack in option to disable MPICH-C++-bindings...
    MPI_NOCXXFLAGS="-DMPICH_SKIP_MPICXX"
}

mpi_getmpich2flags() {
    # use special commands to extract options      
    mpi_getflags "-show" "-c"
    MPI_CPPFLAGS="$retval"
    # remove implicitly set -c
    mpi_remove "$MPI_CPPFLAGS" '-c'
    MPI_CPPFLAGS="$retval"
    
    # get linker options
    mpi_getflags "-show" "-o"
    MPI_LDFLAGS="$retval"
    # strip -o option
    mpi_remove "$MPI_LDFLAGS" "-o"
    MPI_LDFLAGS="$retval"
    #strip MPI_CPPFLAGS (which are included for mpich2 on jugene)
    enc=`echo "$MPI_CPPFLAGS" | sed -e 's/\\//\\\\\\//g'`
    MPI_LDFLAGS=`echo "$retval" | sed -e "s/$enc / /"`


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
    MPI_VERSION="MPICH"
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
    MPI_VERSION="MPICH2"
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
    MPI_VERSION="OpenMPI"

    mpi_getflags "-showme:compile"
    MPI_CPPFLAGS="$retval"
    mpi_getflags "-showme:link"
    MPI_LDFLAGS="$retval"
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
  if $MPICC -v -c conftest.c > /dev/null 2>&1; then
    mpi_getflags "-v" "-c dummy.c"
    MPI_VERSION=`echo $retval | sed -e 's/for \(MVAPICH[[0-9]][[0-9]]*\)-[[0-9.]][[0-9.]]*/\1/'`
    if (echo $MPI_VERSION | grep ^MVAPICH>/dev/null);then
      ADDFLAGS="-DMPICH_IGNORE_CXX_SEEK"
      mpi_getflags "-compile_info $ADDFLAGS" "dummy.o -o dummy"
      MPI_CPPFLAGS="$retval"
      mpi_getflags "-link_info $ADDFLAGS" "dummy.o -o dummy"
      MPI_LDFLAGS="$retval"
      AC_MSG_RESULT([yes])
      rm -f conftest*
      return 0 
    fi
  fi
  AC_MSG_RESULT([no])
  return 1
}

test_ibmmpi() {
  AC_MSG_CHECKING([for IBM MPI])
  if $MPICC -v -c conftest.c > /dev/null 2>&1; then
    mpi_getflags "-v" "-c dummy.c"
    if (echo $retval | grep '^xl[[cC]]'); then
      MPI_VERSION="IBM MPI"

      # get compilation script
#      AC_LANG_CASE([C],[
#        MPICOMP="$MPICC"
#        dune_mpi_isgnu="$GCC"
#      ],
#      [C++],[
#        MPICOMP="$MPICXX"
#        dune_mpi_isgnu="$GXX"
#      ])
      # mpCC assumes xlc is used...
#      if test x$dune_mpi_isgnu = xyes ; then
#        # change commandline if GNU compiler is used
#        retval=`echo $retval | sed -e 's/\(-b[[^ ]]*\)/-Xlinker \1/g'`
#      fi
      MPI_CPPFLAGS="$retval"
  
      mpi_getflags "-v" "dummy.o -o dummy"

#      if test x$dune_mpi_isgnu = xyes ; then
#        # change commandline if GNU compiler is used
#        retval=`echo $retval | sed -e 's/\(-b[[^ ]]*\)/-Xlinker \1/g'`
#      fi

      MPI_LDFLAGS="$retval"

      AC_MSG_RESULT([yes])
      rm -f conftest*
      return 0  
    fi
  fi
  
  AC_MSG_RESULT([no])
  return 1
}

get_mpiparameters() {
  AC_MSG_NOTICE([Trying to identify the version of MPI compiler $MPICC])

  if test x"$MPI_VERSION" != x; then
    return
  fi 

  test_lam && return
  test_mpich && return
  test_openmpi && return
  test_mvapich && return
  test_mpich2 && return
  test_ibmmpi && return
   
  MPI_VERSION="unknown"
  AC_MSG_ERROR([Could not identify MPI-package! Please send a bugreport and tell us what MPI-package you're using.])
}
])

AC_DEFUN([MPI_CONFIG],[
  AC_REQUIRE([MPI_CONFIG_HELPER])  
  get_mpiparameters;
])
