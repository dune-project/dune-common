#include <config.h>

#include <iostream>
#include <numeric>
#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/dynvector.hh>

int main(int argc, char** argv){
  auto& mpihelper = Dune::MPIHelper::instance(argc, argv);

  auto cc = mpihelper.getCollectiveCommunication();

  // p2p
  if(mpihelper.size() > 1){
    if(mpihelper.rank() == 0){
      auto f = cc.isend(42, 1, 0);
      f.wait();
      int i = 42;
      auto f2 = cc.isend<const int&>(i, 1, 0);
      f2.wait();
    }else if(mpihelper.rank() == 1){
      auto f = cc.irecv(41, 0, 0);
      std::cout << "Rank 1 received " << f.get() << std::endl;
      int j = 41;
      auto f2 = cc.irecv<const int&>(j, 0, 0);
      std::cout << "Rank 1 received " << f2.get() << std::endl;
    }
  }

  int answer;
  if(mpihelper.rank() == 0){
    std::cout << "Broadcast lvalue-reference" << std::endl;
    answer = 42;
  }
  auto f = cc.template ibroadcast(answer, 0);
  f.wait();
  std::cout << "Rank " << mpihelper.rank() << " knows: The answer is " << answer << std::endl;
  if(mpihelper.rank() == 0)
    std::cout << "Broadcast value" << std::endl;
  auto f2 = cc.template ibroadcast(int(answer), 0);
  std::cout << "Rank " << mpihelper.rank() << " knows: The answer is " << f.get() << std::endl;

  Dune::DynamicVector<double> vec(3);
  if(mpihelper.rank() == 0){
    std::cout << "Broadcast vector" << std::endl;
    std::iota(vec.begin(), vec.end(), 41);
  }
  auto f3 = cc.ibroadcast(vec, 0);
  f3.wait();
  std::cout << "Rank " << mpihelper.rank() << " received vector: " << vec << std::endl;

  if(mpihelper.rank() == 0)
    std::cout << "nonb Barrier ==========================" << std::endl;
  auto f4 = cc.ibarrier();
  f4.wait();

  if(mpihelper.rank() == 0){
    std::cout << "nonb gather ===========================" << std::endl;
    auto f = cc.igather(mpihelper.rank(), Dune::DynamicVector<int>(mpihelper.size()), 0);
    std::cout << "Gather result: " << f.get() << std::endl;
  }else{
    cc.igather(mpihelper.rank(), {}, 0).wait();
  }

  if(mpihelper.rank() == 0){
    std::cout << "nonb scatter ===========================" << std::endl;
    std::vector<int> my_buddies(mpihelper.size());
    std::iota(my_buddies.begin(), my_buddies.end(), 42);
    auto f = cc.iscatter(my_buddies, 0, 0);
    std::cout << "Scatter result (Rank " << mpihelper.rank() << "): " << f.get() << std::endl;
  }else{
    auto f = cc.iscatter(std::vector<int>(0), 0, 0);
    std::cout << "Scatter result (Rank " << mpihelper.rank() << "): " << f.get() << std::endl;
  }

  {
    if(mpihelper.rank() == 0)
      std::cout << "nonb allreduce ===========================" << std::endl;
    auto f = cc.iallreduce<std::plus<int>>(mpihelper.rank(), 0);
    std::cout << "Allreduce result on rank " << mpihelper.rank() <<": " << f.get() << std::endl;
  }

  {
    if(mpihelper.rank() == 0)
      std::cout << "nonb allreduce inplace ===========================" << std::endl;
    auto f = cc.iallreduce<std::plus<int>>(Dune::DynamicVector<int>{42, 3+mpihelper.rank()});
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
