#include"config.h"
#include<iostream>
#include<mpi.h>

struct MyDataHandle
{
    typedef std::size_t DataType;

    bool fixedsize()
    {
        return true;
    }
    template<class B>
    void gather(B& buffer, int i)
    {
        std::cout<<"Gathering "<<i<<std::endl;
        buffer.write(i);
        buffer.write(i);
        buffer.write(i);
    }
    template<class B>
    void scatter(B& buffer, int i, int size)
    {
        std::cout<<"Scattering "<<size<<" entries for "<<i<<": ";
        for(;size>0;--size)
        {
            std::size_t index;
            buffer.read(index);
            std::cout<<index<<" ";
        }
        std::cout<<std::endl;
    }
    std::size_t size(int i)
    {
        return 3;
    }
};

#include<dune/common/parallel/variablesizecommunicator.hh>

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    {
        typedef Dune::VariableSizeCommunicator<>::InterfaceMap Interface;
        Dune::InterfaceInformation send, recv;
        send.reserve(6);
        for(std::size_t i=0; i<=10; i+=2)
            send.add(i);

        recv.reserve(6);
        for(std::size_t i=10; i<=10; i-=2)
            recv.add(i);
        Interface inf;
        inf[0]=std::make_pair(send, recv);
        Dune::VariableSizeCommunicator<> comm(MPI_COMM_SELF, inf, 6);
        MyDataHandle handle;
        comm.forward(handle);
        std::cout<<"===================== backward ========================="<<std::endl;
        comm.backward(handle);
    }
    MPI_Finalize();
}
