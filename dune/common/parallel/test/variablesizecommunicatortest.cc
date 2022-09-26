// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>

#include <cstddef>
#include <iostream>
#include <set>
#include <utility>
#include <vector>

#include <mpi.h>

#include <dune/common/parallel/interface.hh>
#include <dune/common/parallel/variablesizecommunicator.hh>

// For each communicated index, send convert the index to a `double` and send
// it three times on the sending side.  On the receiving side, simply print
// the received data.
struct MyDataHandle
{
    std::set<int> dataSendAt;
    std::set<int> dataRecievedAt;

    MyDataHandle(int r)
    : rank(r)
    {
    }
    int rank;

    typedef double DataType;

    bool fixedSize()
    {
        return true;
    }
    void verify(int procs, int start, int end) {
      std::vector<int> indices;

      if(procs==1) {
        for(int k=0;k<=10;k+=2) {
          indices.push_back(k);
        }
      }
      else {
        if(rank && rank < procs) {
          indices.push_back(start-1);
          indices.push_back(start);
        }
        if(rank < procs-1) {
          indices.push_back(end-1);
          indices.push_back(end);
        }
      }

      std::set<int>::iterator it;
      for(int idx : indices) {
        it  = dataSendAt.find(idx);
        if(it == dataSendAt.end()) {
          std::cerr << rank << ": No data send at index " << idx << "!" << std::endl;
          std::abort();
        }
        dataSendAt.erase(it);

        it  = dataRecievedAt.find(idx);
        if(it == dataRecievedAt.end()) {
          std::cerr << rank << ": No data received at index " << idx << "!" << std::endl;
          std::abort();
        }
        dataRecievedAt.erase(it);
      }
      for(const int &i : dataSendAt) {
        std::cerr << rank << ": Unexpected data send at index " << i << "!" << std::endl;
        std::abort();
      }
      for(const int &i : dataRecievedAt) {
        std::cerr << rank << ": Unexpected data received at index " << i << "!" << std::endl;
        std::abort();
      }
    }
    template<class B>
    void gather(B& buffer, int i)
    {
        if(!dataSendAt.insert(i).second) {
          std::cerr << rank << ": Gather() was called twice for index " << i << "!" << std::endl;
          std::abort();
        }

        std::cout<<rank<<": Gathering "<<i<<std::endl;
        double d=i;
        buffer.write(d);
        buffer.write(d);
        buffer.write(d);
    }
    template<class B>
    void scatter(B& buffer, int i, int size)
    {
        if(!dataRecievedAt.insert(i).second) {
          std::cerr << rank << ": Scatter() was called twice for index " << i << "!" << std::endl;
          std::abort();
        }

        std::cout<<rank<<": Scattering "<<size<<" entries for "<<i<<": ";
        if(size != 3) {
          std::cerr << "\n" << rank <<": Number of communicated entries does not match!" << std::endl;
          std::abort();
        }

        for(;size>0;--size)
        {
            double index;
            buffer.read(index);
            std::cout<<index<<" ";
            if(i != index) {
              std::cerr << "\n" << rank << ": Communicated value does not match!" << std::endl;
              std::abort();
            }
        }
        std::cout<<std::endl;
    }
    std::size_t size([[maybe_unused]] int i)
    {
        return 3;
    }
};

struct MyDataHandle1D : public MyDataHandle
{
    MyDataHandle1D(int r) : MyDataHandle(r) {}

    template<class B>
    void scatter(B& buffer, int i, int size)
    {
        if(!dataRecievedAt.insert(i).second) {
          std::cerr << rank << ": Scatter() was called twice for index " << i << "!" << std::endl;
          std::abort();
        }

        std::cout<<rank<<": Scattering "<<size<<" entries for "<<i<<": ";
        if(size != 3) {
          std::cerr << "\n" << rank <<": Number of communicated entries does not match!" << std::endl;
          std::abort();
        }

        for(;size>0;--size)
        {
            double index;
            buffer.read(index);
            std::cout<<index<<" ";
            if(10-i != index) {
              std::cerr << "\n" << rank << ": Communicated value does not match!" << std::endl;
              std::abort();
            }
        }
        std::cout<<std::endl;
    }
};

// On the sending side, for each index to send, send between 0 and 4 numbers
// (precisely: `index % 5` numbers).  The first number is the index converted
// to `double`, incrementing by one for each consecutive number.  On the
// receiving side just print the received numbers.
struct VarDataHandle
{
    std::set<int> dataSendAt;
    std::set<int> dataRecievedAt;

    VarDataHandle(int r)
    : rank(r)
    {}
    int rank;
    typedef double DataType;
    bool fixedSize()
    {
        return false;
    }
    void verify(int procs, int start, int end) {
      std::vector<int> indices;
      if(procs==1) {
        for(int k=0;k<=10;k+=2) {
          indices.push_back(k);
        }
      }
      else {
        if(rank && rank < procs) {
          indices.push_back(start-1);
          indices.push_back(start);
        }
        if(rank < procs-1) {
          indices.push_back(end-1);
          indices.push_back(end);
        }
      }

      std::set<int>::iterator it;
      for(int idx : indices) {
        it  = dataSendAt.find(idx);
        if(it == dataSendAt.end()) {
          std::cerr << rank << ": No data send at index " << idx << "!" << std::endl;
          std::abort();
        }
        dataSendAt.erase(it);

        it  = dataRecievedAt.find(idx);
        if(it == dataRecievedAt.end() && idx%5) {
          std::cerr << rank << ": No data received at index " << idx << "!" << std::endl;
          std::abort();
        }
        else if(it != dataRecievedAt.end()) {
          dataRecievedAt.erase(it);
        }
      }
      for(const int &i : dataSendAt) {
        std::cerr << rank << ": Unexpected data send at index " << i << "!" << std::endl;
        std::abort();
      }
      for(const int &i : dataRecievedAt) {
        std::cerr << rank << ": Unexpected data received at index " << i << "!" << std::endl;
        std::abort();
      }
    }
    template<class B>
    void gather(B& buffer, int i)
    {
        if(!dataSendAt.insert(i).second) {
          std::cerr << rank << ": Gather() was called twice for index " << i << "!" << std::endl;
          std::abort();
        }

        std::size_t s=i%5;
        std::cout<<rank<<": Gathering "<<s<<" entries for index "<<i<<std::endl;
        for(std::size_t j=0; j<s; j++)
            buffer.write(static_cast<double>(i+j));
    }
    template<class B>
    void scatter(B& buffer, int i, int size)
    {
        if(!dataRecievedAt.insert(i).second) {
          std::cerr << rank << ": Scatter() was called twice for index " << i << "!" << std::endl;
          std::abort();
        }

        std::cout<<rank<<": Scattering "<<size<<" entries for "<<i<<": ";
        if(size != i%5) {
          std::cerr << "\n" << rank <<": Number of communicated entries does not match!" << std::endl;
          std::abort();
        }

        for(int k=0; k<size; k++)
        {
            double index;
            buffer.read(index);
            std::cout<<index<<" ";
            if(index != i+k) {
              std::cerr << "\n" << rank << ": Communicated value does not match!" << std::endl;
              std::abort();
            }
        }
        std::cout<<std::endl;
    }
    std::size_t size(int i)
    {
        return i%5;
    }

};

struct VarDataHandle1D : public VarDataHandle
{
    VarDataHandle1D(int r) : VarDataHandle(r) {}

    template<class B>
    void scatter(B& buffer, int i, int size)
    {
        if(!dataRecievedAt.insert(i).second) {
          std::cerr << rank << ": Scatter() was called twice for index " << i << "!" << std::endl;
          std::abort();
        }

        std::cout<<rank<<": Scattering "<<size<<" entries for "<<i<<": ";
        if(size != (10-i)%5) {
          std::cerr << "\n" << rank <<": Number of communicated entries does not match!" << std::endl;
          std::abort();
        }

        for(int k=0; k<size; k++)
        {
            double index;
            buffer.read(index);
            std::cout<<index<<" ";
            if(index != 10-i+k) {
              std::cerr << "\n" << rank << ": Communicated value does not match!" << std::endl;
              std::abort();
            }
        }
        std::cout<<std::endl;
    }
};

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    int procs, rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);
    if(procs==1)
    {
        // Invent a consecutive index set with 11 indices [0, 10].  Set every
        // even index to communicate with ourself.  Then use the data handles
        // defined at the top of this file to do some test communications.
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
        MyDataHandle1D handle(0);
        comm.forward(handle);
        handle.verify(procs, 0, 0);
        std::cout<<"===================== backward ========================="<<std::endl;
        comm.backward(handle);
        handle.verify(procs, 0, 0);
        std::cout<<"================== variable size ======================="<<std::endl;
        VarDataHandle1D vhandle(0);
        comm.forward(vhandle);
        vhandle.verify(procs, 0, 0);
        std::cout<<"===================== backward ========================="<<std::endl;
        comm.backward(vhandle);
        vhandle.verify(procs, 0, 0);
    }
    else
    {
        // We also want to check the case where the interface is empty on some
        // processes. Therefore we artificially lower the number of processes
        // if it is larger than two. Thus the last rank will not send anything
        // and we check for deadlocks.
        if(procs>2)
            --procs;

        // Partition a consecutive set of indices among all active ranks
        // (where the final rank possibly excluded above is considered
        // inactive).  Set up interfaces so each rank communicates with its
        // predecessors at the two indices next to the common partition
        // boundary, and likewise for the successor.  Then use the data
        // handles defined at the top of this file to do some test
        // communications.
        int N=100000; // number of indices
        int num_per_proc=N/procs;
        // start is our first index, end is one-past our last index.
        int start, end;
        if(rank<N%procs)
        {
            // if the #active ranks does not divide #indices, lower ranks get
            // an additional index in their range
            start=rank*(num_per_proc+1);
            end=(rank+1)*(num_per_proc+1);
        }
        else
        {
            start=(N%procs)+rank*(num_per_proc);
            end=start+num_per_proc;
        }
        // sanity check
        if(rank==procs-1)
            assert(N==end);
        typedef Dune::VariableSizeCommunicator<>::InterfaceMap Interface;
        Interface inf;
        if(rank && rank<procs) // rank==procs might hold and produce a deadlock otherwise!
        {
            // left interface: communicate our first index and our
            // predecessor's last index with our predecessor
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
            // right interface: communicate our last index and our successor's
            // first index with our successor
            Dune::InterfaceInformation send, recv;
            send.reserve(2);
            recv.reserve(2);
            send.add(end-1);
            send.add(end);
            recv.add(end-1);
            recv.add(end);
            inf[rank+1]=std::make_pair(send, recv);
        }
        // report inactive rank
        if(rank==procs)
            std::cout<<" rank "<<rank<<" has empty interface "<<inf.size()<<std::endl;

        Dune::VariableSizeCommunicator<> comm(MPI_COMM_WORLD, inf, 6);
        MyDataHandle handle(rank);
        comm.forward(handle);
        MPI_Barrier(MPI_COMM_WORLD);
        handle.verify(procs, start, end);
        MPI_Barrier(MPI_COMM_WORLD);
        if(rank==0)
            std::cout<<"===================== backward ========================="<<std::endl;
        MPI_Barrier(MPI_COMM_WORLD);
        comm.backward(handle);
        MPI_Barrier(MPI_COMM_WORLD);
        handle.verify(procs, start, end);
        MPI_Barrier(MPI_COMM_WORLD);
        if(rank==0)
            std::cout<<"================== variable size ======================="<<std::endl;
        MPI_Barrier(MPI_COMM_WORLD);
        VarDataHandle vhandle(rank);
        MPI_Barrier(MPI_COMM_WORLD);
        comm.forward(vhandle);
        MPI_Barrier(MPI_COMM_WORLD);
        vhandle.verify(procs, start, end);
        MPI_Barrier(MPI_COMM_WORLD);
        if(rank==0)
            std::cout<<"===================== backward ========================="<<std::endl;
        MPI_Barrier(MPI_COMM_WORLD);
        comm.backward(vhandle);
        MPI_Barrier(MPI_COMM_WORLD);
        vhandle.verify(procs, start, end);
    }

    MPI_Finalize();

    return 0;
}
