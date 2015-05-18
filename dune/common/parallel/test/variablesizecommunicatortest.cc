#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include<iostream>
#if HAVE_MPI
#include<mpi.h>
#endif

#include <dune/common/unused.hh>

struct MyDataHandle
{
    MyDataHandle(int r)
    : rank(r)
    {
    }
    int rank;

    typedef double DataType;

    bool fixedsize()
    {
        return true;
    }
    template<class B>
    void gather(B& buffer, int i)
    {
        std::cout<<rank<<": Gathering "<<i<<std::endl;
        double d=i;
        buffer.write(d);
        buffer.write(d);
        buffer.write(d);
    }
    template<class B>
    void scatter(B& buffer, int i, int size)
    {
        std::cout<<rank<<": Scattering "<<size<<" entries for "<<i<<": ";
        for(;size>0;--size)
        {
            double index;
            buffer.read(index);
            std::cout<<index<<" ";
        }
        std::cout<<std::endl;
    }
    std::size_t size(int i)
    {
        DUNE_UNUSED_PARAMETER(i);
        return 3;
    }
};

struct VarDataHandle
{
    VarDataHandle(int r)
    : rank(r)
    {}
    int rank;
    typedef double DataType;
    bool fixedsize()
    {
        return false;
    }

    template<class B>
    void gather(B& buffer, int i)
    {
        std::size_t s=i%5;
        std::cout<<rank<<": Gathering "<<s<<" entries for index "<<i<<std::endl;
        for(std::size_t j=0; j<s; j++)
            buffer.write(static_cast<double>(i+j));
    }
    template<class B>
    void scatter(B& buffer, int i, int size)
    {
        std::cout<<rank<<": Scattering "<<size<<" entries for "<<i<<": ";
        for(;size>0;--size)
        {
            double index;
            buffer.read(index);
            std::cout<<index<<" ";
        }
        std::cout<<std::endl;
    }
    std::size_t size(int i)
    {
        return i%5;
    }

};

#include<dune/common/parallel/variablesizecommunicator.hh>

int main(int argc, char** argv)
{
#if HAVE_MPI
    MPI_Init(&argc, &argv);
    int procs, rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);
    if(procs==1)
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
        MyDataHandle handle(0);
        comm.forward(handle);
        std::cout<<"===================== backward ========================="<<std::endl;
        comm.backward(handle);
        std::cout<<"================== variable size ======================="<<std::endl;
        VarDataHandle vhandle(0);
        comm.forward(vhandle);
        std::cout<<"===================== backward ========================="<<std::endl;
        comm.backward(vhandle);
    }
    else
    {
        // We also want to check the case where the interface is empty on some
        // processes. Therefore we artificially lower the numer of processes
        // if it is larger than two. Thus the last rank will not send anything
        // and we check for deadlocks.
        if(procs>2)
            --procs;
        int N=100000;
        int num_per_proc=N/procs;
        int start, end;
        if(rank<N%procs)
        {
            start=rank*(num_per_proc+1);
            end=(rank+1)*(num_per_proc+1);
        }
        else
        {
            start=(N%procs)+rank*(num_per_proc);
            end=start+num_per_proc;
        }
        if(rank==procs-1)
            assert(N==end);
        typedef Dune::VariableSizeCommunicator<>::InterfaceMap Interface;
        Interface inf;
        if(rank && rank<procs) // rank==procs might hold and produce a deadlock otherwise!
        {
            Dune::InterfaceInformation send, recv;
            send.reserve(2);
            recv.reserve(2);
            send.add(start-1);
            send.add(start);
            recv.add(start-1);
            recv.add(start);
            inf[rank-1]=std::make_pair(send, recv);
        }
        if(rank<procs-1)
        {

            Dune::InterfaceInformation send, recv;
            send.reserve(2);
            recv.reserve(2);
            send.add(end-1);
            send.add(end);
            recv.add(end-1);
            recv.add(end);
            inf[rank+1]=std::make_pair(send, recv);
        }
        if(rank==procs)
            std::cout<<" rank "<<rank<<" has empty interface "<<inf.size()<<std::endl;

        Dune::VariableSizeCommunicator<> comm(MPI_COMM_WORLD, inf, 6);
        MyDataHandle handle(rank);
        comm.forward(handle);
        MPI_Barrier(MPI_COMM_WORLD);
        if(rank==0)
            std::cout<<"===================== backward ========================="<<std::endl;
        MPI_Barrier(MPI_COMM_WORLD);
        comm.backward(handle);
        MPI_Barrier(MPI_COMM_WORLD);
        if(rank==0)
            std::cout<<"================== variable size ======================="<<std::endl;
        MPI_Barrier(MPI_COMM_WORLD);

        VarDataHandle vhandle(rank);
        MPI_Barrier(MPI_COMM_WORLD);
        comm.forward(vhandle);
        MPI_Barrier(MPI_COMM_WORLD);
        if(rank==0)
            std::cout<<"===================== backward ========================="<<std::endl;
        comm.backward(vhandle);
    }

    MPI_Finalize();
#else
    std::cerr<<"Skipping as MPI seems not be available"<<std::endl;
    return 77;
#endif
}
