// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <config.h>

#include <iostream>
#include <numeric>
#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/dynvector.hh>

namespace Dune {
  template<class Dummy>
  struct MPIData<const int&, Dummy>
  {
    static_assert(Dune::AlwaysFalse<Dummy>::value, "MPIData of reference type should not be used!");
    // MPIData of reference type should not be used!
    // This struct should never be used it just
    // exists to generate a compiler error
  };
}


int main(int argc, char** argv){
  auto& mpihelper = Dune::MPIHelper::instance(argc, argv);

  auto cc = mpihelper.getCommunication();

  // p2p
  if(mpihelper.size() > 1){
    if(mpihelper.rank() == 0){
      Dune::Future<int> f = cc.isend(42, 1, 0);
      f.wait();
      int i = 42;
      Dune::Future<const int&> f2 = cc.isend<const int&>(i, 1, 0);
      f2.wait();
    }else if(mpihelper.rank() == 1){
      Dune::Future<int> f = cc.irecv(41, 0, 0);
      std::cout << "Rank 1 received " << f.get() << std::endl;
      int j = 41;
      Dune::Future<int&> f2 = cc.irecv<int&>(j, 0, 0);
      std::cout << "Rank 1 received " << f2.get() << std::endl;
    }
  }

  int answer;
  if(mpihelper.rank() == 0){
    std::cout << "Broadcast lvalue-reference" << std::endl;
    answer = 42;
  }
  Dune::Future<int> f = cc.template ibroadcast(answer, 0);
  f.wait();
  std::cout << "Rank " << mpihelper.rank() << " knows: The answer is " << answer << std::endl;
  if(mpihelper.rank() == 0)
    std::cout << "Broadcast value" << std::endl;
  Dune::Future<int> f2 = cc.template ibroadcast(int(answer), 0);
  std::cout << "Rank " << mpihelper.rank() << " knows: The answer is " << f2.get() << std::endl;

  Dune::DynamicVector<double> vec(3);
  if(mpihelper.rank() == 0){
    std::cout << "Broadcast vector" << std::endl;
    std::iota(vec.begin(), vec.end(), 41);
  }
  Dune::Future<Dune::DynamicVector<double>> f3 = cc.ibroadcast(vec, 0);
  f3.wait();
  std::cout << "Rank " << mpihelper.rank() << " received vector: " << vec << std::endl;

  if(mpihelper.rank() == 0)
    std::cout << "nonb Barrier ==========================" << std::endl;
  Dune::Future<void> f4 = cc.ibarrier();
  f4.wait();

  if(mpihelper.rank() == 0){
    std::cout << "nonb gather ===========================" << std::endl;
    Dune::Future<Dune::DynamicVector<int>> f = cc.igather(mpihelper.rank() + 42, Dune::DynamicVector<int>(mpihelper.size()), 0);
    std::cout << "Gather result: " << f.get() << std::endl;
  }else{
    cc.igather(mpihelper.rank(), {}, 0).wait();
  }

  if(mpihelper.rank() == 0){
    std::cout << "nonb scatter ===========================" << std::endl;
    std::vector<int> my_buddies(mpihelper.size());
    std::iota(my_buddies.begin(), my_buddies.end(), 42);
    Dune::Future<int> f = cc.iscatter(my_buddies, 0, 0);
    std::cout << "Scatter result (Rank " << mpihelper.rank() << "): " << f.get() << std::endl;
  }else{
    Dune::Future<int> f = cc.iscatter(std::vector<int>(0), 0, 0);
    std::cout << "Scatter result (Rank " << mpihelper.rank() << "): " << f.get() << std::endl;
  }

  {
    if(mpihelper.rank() == 0)
      std::cout << "nonb allreduce ===========================" << std::endl;
    Dune::Future<int> f = cc.iallreduce<std::plus<int>>(mpihelper.rank()+4, 0);
    std::cout << "Allreduce result on rank " << mpihelper.rank() <<": " << f.get() << std::endl;
  }

  {
    if(mpihelper.rank() == 0)
      std::cout << "nonb allreduce inplace ===========================" << std::endl;
    Dune::Future<Dune::DynamicVector<int>> f = cc.iallreduce<std::plus<int>>(Dune::DynamicVector<int>{42, 3+mpihelper.rank()});
    std::cout << "Allreduce result on rank " << mpihelper.rank() <<": " << f.get() << std::endl;
  }

  {
    if(mpihelper.rank() == 0)
      std::cout << "check for MPI_SUM with double&  ===========================" << std::endl;
    double answer = 42;
    auto f = cc.iallreduce<std::plus<double>>(answer);
    std::cout << "Allreduce result on rank " << mpihelper.rank() <<": " << f.get() << std::endl;
  }

  // that's wrong, MPIFuture will hold a dangeling reference:
  // Dune::MPIFuture<int&> g;
  // {
  //   int i = 42;
  //   g = cc.iallreduce<std::plus<int>>(i);
  // }
  // g.wait();
  return 0;
}
