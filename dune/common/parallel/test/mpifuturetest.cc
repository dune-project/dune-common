// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpiguard.hh>

template<class F>
auto testSemantics(F f){
  if(!f.valid())
    DUNE_THROW(Dune::InvalidStateException, "Future not valid");
  f.wait();
  if(!f.ready())
    DUNE_THROW(Dune::Exception, "Not ready after wait()");
  auto result = f.get();
  if(f.valid())
    DUNE_THROW(Dune::Exception, "Valid after get()");
  return result;
}

template<class FG>
auto test_when_all(FG future_generator){
  // when all (tuple)
  auto f = future_generator();
  auto g = future_generator();
  auto wall = Dune::when_all(std::move(f), std::move(g));
  auto result = testSemantics(std::move(wall));
  testSemantics(std::move(std::get<0>(result)));
  testSemantics(std::move(std::get<1>(result)));
  f = future_generator();
  f.wait();
  wall = Dune::when_all(std::move(f), future_generator());
  result = testSemantics(std::move(wall));
  testSemantics(std::move(std::get<0>(result)));
  testSemantics(std::move(std::get<1>(result)));
  f = future_generator();
  g= future_generator();
  f.wait();
  g.wait();
  wall = Dune::when_all(std::move(f), std::move(g));
  result = testSemantics(std::move(wall));
  testSemantics(std::move(std::get<0>(result)));
  testSemantics(std::move(std::get<1>(result)));
  // when all (vector)
  std::vector<decltype(f)> vec(3);
  for(auto& h : vec)
    h = future_generator();
  auto wall2 = Dune::when_all(vec.begin(), vec.end());
  auto result2 = testSemantics(std::move(wall2));
  for(auto& h : result2)
    testSemantics(std::move(h));
  for(auto& h : vec)
    h = future_generator();
  vec[0].wait();
  wall2 = Dune::when_all(vec.begin(), vec.end());
  result2 = testSemantics(std::move(wall2));
  for(auto& h : result2)
    testSemantics(std::move(h));
  for(auto& h : vec)
    h = future_generator();
  for(auto& h : vec)
    h.wait();
  wall2 = Dune::when_all(vec.begin(), vec.end());
  result2 = testSemantics(std::move(wall2));
  for(auto& h : result2)
    testSemantics(std::move(h));
  // when_all tuple of lvalue-references
  f = future_generator();
  g = future_generator();
  auto wall3 = Dune::when_all(f, g);
  auto result3 = testSemantics(std::move(wall3));
  testSemantics(std::move(std::get<0>(result3)));
  testSemantics(std::move(std::get<1>(result3)));
  f = future_generator();
  f.wait();
  g = future_generator();
  auto wall4 = Dune::when_all(f, g);
  auto result4 = testSemantics(wall4);
  testSemantics(std::move(std::get<0>(result4)));
  testSemantics(std::move(std::get<1>(result4)));
  f = future_generator();
  g = future_generator();
  f.wait();
  g.wait();
  auto wall5 = Dune::when_all(f, g);
  auto result5 = testSemantics(wall5);
  testSemantics(std::move(std::get<0>(result5)));
  testSemantics(std::move(std::get<1>(result5)));
}

template<class FG>
auto test_when_any(FG future_generator){
  // when any (tuple)
  auto f = future_generator();
  auto g = future_generator();
  auto wany = Dune::when_any(std::move(f), std::move(g));
  auto result = testSemantics(std::move(wany));
  testSemantics(std::move(std::get<0>(result.futures)));
  testSemantics(std::move(std::get<1>(result.futures)));
  f = future_generator();
  f.wait();
  wany = Dune::when_any(std::move(f), future_generator());
  result = testSemantics(std::move(wany));
  testSemantics(std::move(std::get<0>(result.futures)));
  testSemantics(std::move(std::get<1>(result.futures)));
  f = future_generator();
  g= future_generator();
  f.wait();
  g.wait();
  wany = Dune::when_any(std::move(f), std::move(g));
  result = testSemantics(std::move(wany));
  testSemantics(std::move(std::get<0>(result.futures)));
  testSemantics(std::move(std::get<1>(result.futures)));
  // when any (vector)
  std::vector<decltype(f)> vec(3);
  for(auto& h : vec)
    h = future_generator();
  auto wany2 = Dune::when_any(vec.begin(), vec.end());
  auto result2 = testSemantics(std::move(wany2));
  for(auto& h : result2.futures)
    testSemantics(std::move(h));
  for(auto& h : vec)
    h = future_generator();
  vec[0].wait();
  wany2 = Dune::when_any(vec.begin(), vec.end());
  result2 = testSemantics(std::move(wany2));
  for(auto& h : result2.futures)
    testSemantics(std::move(h));
  for(auto& h : vec)
    h = future_generator();
  for(auto& h : vec)
    h.wait();
  wany2 = Dune::when_any(vec.begin(), vec.end());
  result2 = testSemantics(std::move(wany2));
  for(auto& h : result2.futures)
    testSemantics(std::move(h));
  // when_any tuple of lvalue-references
  f = future_generator();
  g = future_generator();
  auto wany3 = Dune::when_any(f, g);
  auto result3 = testSemantics(std::move(wany3));
  testSemantics(std::move(std::get<0>(result3.futures)));
  testSemantics(std::move(std::get<1>(result3.futures)));
  f = future_generator();
  f.wait();
  g = future_generator();
  auto wany4 = Dune::when_any(f, g);
  g.wait();
  auto result4 = testSemantics(wany4);
  testSemantics(std::move(std::get<0>(result4.futures)));
  testSemantics(std::move(std::get<1>(result4.futures)));
  f = future_generator();
  g = future_generator();
  f.wait();
  g.wait();
  auto wany5 = Dune::when_any(f, g);
  auto result5 = testSemantics(wany5);
  testSemantics(std::move(std::get<0>(result5.futures)));
  testSemantics(std::move(std::get<1>(result5.futures)));
}

template<class FG>
void testFuture(FG future_generator){
  testSemantics(future_generator());
  auto f = future_generator();
  f.wait();
  testSemantics(std::move(f));

  test_when_all(future_generator);
  test_when_any(future_generator);
}

int main(int argc, char** argv){
  Dune::MPIHelper& helper = Dune::MPIHelper::instance(argc, argv);

  auto comm = helper.getCommunicator();
  int rank = comm.rank();

  auto cc = helper.getCollectiveCommunication();

  try{
    testFuture([&](){
        return cc.iallreduce<std::plus<int>>(rank);
    });
  }catch(Dune::Exception& e){
    std::cout << "Caught exception: " << e.what() << std::endl;
  }

  return 0;
}
