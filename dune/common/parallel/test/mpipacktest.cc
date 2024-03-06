// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#include <iostream>
#include <map>
#include <vector>

#include <dune/common/classname.hh>
#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpipack.hh>
#include <dune/common/test/testsuite.hh>

constexpr int TAG = 42;

template<typename Comm>
auto testSync(Comm comm)
{
  Dune::TestSuite suite("testSync");

  int rank = comm.rank();
  // we receive from left and right
  int src  = (rank - 1 + comm.size()) % comm.size();
  int dest = (rank + 1) % comm.size();

  // send
  {
    Dune::MPIPack pack(comm);
    pack << 3 << comm.rank();
    pack << std::vector<int>{4711, 42};
    comm.send(pack, dest, TAG);
  }
  // recv
  {
    Dune::MPIPack pack = comm.rrecv(Dune::MPIPack(comm), src, TAG);
    int drei; pack >> drei;
    int rank_src; pack >> rank_src;
    std::vector<int> vec;
    pack >> vec;
    suite.check(drei==3) << "received wrong value";
    suite.check(rank_src==src) << "received wrong value";
    suite.check(vec.size() == 2) << "vector has wrong size!";
    suite.check(vec[0] == 4711 && vec[1] == 42) << "vector contains wrong values!";
  }
  return suite;
}

template<typename Comm>
auto testASync(Comm comm)
{
  Dune::TestSuite suite("testASync");

  int rank = comm.rank();
  // we receive from left and right
  int src  = (rank - 1 + comm.size()) % comm.size();
  int dest = (rank + 1) % comm.size();

  using RecvFuture = Dune::MPIFuture<Dune::MPIPack>;
  using SendFuture = Dune::MPIFuture<Dune::MPIPack>;

  std::map<int, SendFuture> sendFutures;
  std::map<int, RecvFuture> recvFutures;

  // recv async
  {
    const int reserve = 100;
    auto future = comm.irecv(Dune::MPIPack(comm,reserve), src, TAG);
    // MPIPack is non-copyable, thus we can't use sendFutures[dest] = ...;
    recvFutures.emplace(src, std::move(future));
    // Alternative: sendFutures.insert( std::make_pair(dest, std::move(future)) );
  }
  // send async
  {
    Dune::MPIPack pack(comm);
    pack << 3 << comm.rank();
    pack << std::vector<int>{4711, 42};

    auto future = comm.isend(std::move(pack), dest, TAG);
    // MPIPack is non-copyable, thus we can't use sendFutures[dest] = ...;
    sendFutures.emplace(dest, std::move(future));
    // Alternative: sendFutures.insert( std::make_pair(dest, std::move(future)) );
  }
  // recv
  for (auto & [rank, future] : recvFutures)
  {
    Dune::MPIPack pack = future.get();
    int drei; pack >> drei;
    int rank_src; pack >> rank_src;
    std::vector<int> vec;
    pack >> vec;
    suite.check(drei==3) << "received wrong value";
    suite.check(rank_src==rank) << "received wrong value";
    suite.check(vec.size() == 2) << "vector has wrong size!";
    suite.check(vec[0] == 4711 && vec[1] == 42) << "vector contains wrong values!";
  }

  // wait for send operations to finish
  for (auto & [rank, future] : sendFutures)
    future.wait();

  return suite;
}

template<typename Comm>
auto testASyncVector(Comm comm)
{
  Dune::TestSuite suite("testASync");

  int rank = comm.rank();
  // we receive from left and right
  int src  = (rank - 1 + comm.size()) % comm.size();
  int dest = (rank + 1) % comm.size();

  using RecvFuture = Dune::MPIFuture<std::vector<int>>;
  using SendFuture = Dune::MPIFuture<std::vector<int>>;

  SendFuture sendFuture;
  RecvFuture recvFuture;

  // recv async
  {
    const int reserve = 2; // size on the receiving side...
    recvFuture = comm.irecv(std::vector<int>(reserve), src, TAG+src);
  }
  // send async
  {
    std::vector<int> vec{4711, 42}; // we have to keep the vector until send has finished
    sendFuture = comm.isend(std::move(vec), dest, TAG+rank);
  }
  // recv
  {
    std::cout << rank << " reading future" << std::endl;
    std::vector<int> vec = recvFuture.get();
    std::cout << rank << " done" << std::endl;
    suite.check(vec.size() == 2) << "vector has wrong size!";
    suite.check(vec[0] == 4711 && vec[1] == 42) << "vector contains wrong values!";
  }
  // wait for send to finish
  sendFuture.wait();

  return suite;
}

int main(int argc, char** argv){
  Dune::MPIHelper& helper = Dune::MPIHelper::instance(argc, argv);
  Dune::TestSuite suite;
  auto comm = helper.getCommunication();

  suite.subTest(testSync(comm));

  suite.subTest(testASyncVector(comm));

  suite.subTest(testASync(comm));

  return suite.exit();
}
