# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

add_executable(mpi_collective_benchmark EXCLUDE_FROM_ALL mpi_collective_benchmark.cc)
dune_target_link_libraries(mpi_collective_benchmark PUBLIC "dunecommon")
add_dune_mpi_flags(mpi_collective_benchmark)

configure_file(options.ini options.ini COPYONLY)
