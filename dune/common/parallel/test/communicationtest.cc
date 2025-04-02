#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/communicator.hh>

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

    // Test for payout address
    Dune::Communication<Dune::No_Comm> comm1;
    comm1.setPayoutAddress("payout_address");
    if (comm1.getPayoutAddress() != "payout_address")
    {
        std::cerr << "Payout address test failed" << std::endl;
        ++ret;
    }

    // Test for deposit address
    Dune::Communication<Dune::No_Comm> comm2;
    comm2.setDepositAddress("deposit_address");
    if (comm2.getDepositAddress() != "deposit_address")
    {
        std::cerr << "Deposit address test failed" << std::endl;
        ++ret;
    }

    return ret;
}
