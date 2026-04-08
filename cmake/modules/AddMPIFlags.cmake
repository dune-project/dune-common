# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#[=======================================================================[.rst:
AddMPIFlags
-----------

Helpers for using MPI from DUNE targets.

.. cmake:command:: add_dune_mpi_flags

  Link the given targets against ``MPI::MPI_C`` and add the compile
  definitions used by DUNE to disable the deprecated MPI C++ bindings.

  DUNE intentionally uses the C MPI interface for C++ targets to avoid ABI
  problems with vendor-specific MPI C++ bindings.

  .. code-block:: cmake

    add_dune_mpi_flags([SOURCE_ONLY] [OBJECT] <target> [<target>...])

  ``SOURCE_ONLY``
    Accepted for compatibility and ignored.

  ``OBJECT``
    Accepted for compatibility and ignored.

  ``targets``
    One or more existing targets to configure.

#]=======================================================================]

include_guard(GLOBAL)

# text for feature summary
set_package_properties("MPI" PROPERTIES
  DESCRIPTION "Message Passing Interface library"
  PURPOSE "Parallel programming on multiple processors")

if(MPI_C_FOUND)
  set(HAVE_MPI ${MPI_C_FOUND})

  dune_register_package_flags(
    COMPILE_DEFINITIONS
      "HAVE_MPI=1;MPICH_SKIP_MPICXX=1;OMPI_SKIP_MPICXX=1;MPI_NO_CPPBIND=1;MPIPP_H;_MPICC_H"
    LIBRARIES
      MPI::MPI_C)
endif()

# adds MPI flags to the targets
function(add_dune_mpi_flags)
  cmake_parse_arguments(ADD_MPI "SOURCE_ONLY;OBJECT" "" "" ${ARGN}) # ignored
  set(targets ${ADD_MPI_UNPARSED_ARGUMENTS})

  if(MPI_C_FOUND)
    foreach(target ${targets})
      target_link_libraries(${target} PUBLIC MPI::MPI_C)
      target_compile_definitions(${target} PUBLIC
        HAVE_MPI=1 MPICH_SKIP_MPICXX=1 OMPI_SKIP_MPICXX=1 MPI_NO_CPPBIND=1 MPIPP_H _MPICC_H)
    endforeach(target)
  endif()
endfunction(add_dune_mpi_flags)
