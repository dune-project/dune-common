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

    Dune::MPIHelper::MPICommunicator comm = Dune::MPIHelper::getCommunication();

#if HAVE_MPI
    if (MPI_COMM_SELF !=  Dune::MPIHelper::getLocalCommunication())
    {
        std::cerr<<"Dune::MPIHelper::getLocalCommunication() gives wrong result"<<std::endl;
        ++ret;
    }
    if (MPI_COMM_WORLD !=  Dune::MPIHelper::getCommunication())
    {
        std::cerr<<"Dune::MPIHelper::getCommunication() gives wrong result"<<std::endl;
        ++ret;
    }
#endif
    return ret;
}
