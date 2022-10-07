// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>

#include <dune/common/parallel/mpihelper.hh>

int main(int argc, char** argv)
{

#ifdef MPIHELPER_PREINITIALIZE
#if HAVE_MPI
  MPI_Init(&argc, &argv);
#endif
#endif

  typedef Dune::MPIHelper Helper;

  {
    Helper& mpi = Helper::instance(argc, argv);

    [[maybe_unused]] Helper::MPICommunicator comm = mpi.getCommunicator();
    comm= mpi.getCommunicator();
  }

  {
    Helper& mpi = Helper::instance(argc, argv);

    [[maybe_unused]] Helper::MPICommunicator comm = mpi.getCommunicator();
    comm= mpi.getCommunicator();

#ifdef MPIHELPER_PREINITIALIZE
#if HAVE_MPI
    MPI_Finalize();
#endif
#endif
  }
  std::cout << "We are at the end!"<<std::endl;

}
