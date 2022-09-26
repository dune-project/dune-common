// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <dune/common/parallel/mpihelper.hh>
int main(int argc, char** argv)
{
    Dune::MPIHelper::instance(argc, argv);

    int ret = 0;
    Dune::No_Comm nc1, nc2;

    if ( !(nc1 == nc2))
    {
        std::cerr << "operator==: No_Comms need to compare equal"<<std::endl;
        ++ret;
    }
    if ( nc1 != nc2)
    {
        std::cerr << "operator!=: No_Comms need to compare equal"<<std::endl;
        ++ret;
    }

    if (Dune::FakeMPIHelper::getCommunicator() != Dune::FakeMPIHelper::getLocalCommunicator())
    {
        std::cerr << "FakeMPIHelper::getCommunicator() and FakeMPIHelper::getLocalCommunicator()) should be equal"<<std::endl;
        ++ret;
    }

    nc1 = Dune::FakeMPIHelper::getCommunication();

    [[maybe_unused]] Dune::MPIHelper::MPICommunicator comm = Dune::MPIHelper::getCommunication();

#if HAVE_MPI
    if (MPI_COMM_SELF !=  Dune::MPIHelper::getLocalCommunicator())
    {
        std::cerr<<"Dune::MPIHelper::getLocalCommunicator() gives wrong result"<<std::endl;
        ++ret;
    }
    if (MPI_COMM_WORLD !=  Dune::MPIHelper::getCommunicator())
    {
        std::cerr<<"Dune::MPIHelper::getCommunicator() gives wrong result"<<std::endl;
        ++ret;
    }
#endif
    return ret;
}
