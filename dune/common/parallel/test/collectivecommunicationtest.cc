// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpipack.hh>

template<class CC>
void testBarrier(CC& cc){
  // blocking
  cc.barrier();
  // non-blocking
  auto f = cc.ibarrier();
  f.wait();
}

template<class CC, class T>
void testBroadcast(CC& cc, const T& data, const T& init = {}){
  { // blocking
    if(cc.rank() == 0){
      T buf(data);
      cc.broadcast(buf, 0);
    }else{
      T buf(init);
      if(Dune::Span<T>::dynamic_size)
        Dune::Span<T>(buf).resize(Dune::Span<const T>(data).size());
      cc.broadcast(buf, 0);
      if(buf != data)
        DUNE_THROW(Dune::Exception, "Blocking Broadcast failed!");
    }
  }
  { // non-blocking
    Dune::Future<T> f;
    if(cc.rank() == 0)
      f = cc.ibroadcast(data, 0);
    else{
      T buf(init);
      if(Dune::Span<T>::dynamic_size)
        Dune::Span<T>(buf).resize(Dune::Span<const T>(data).size());
      f = cc.ibroadcast(std::move(buf), 0);
    }
    T buf = f.get();
    if(buf != data)
      DUNE_THROW(Dune::Exception, "Non-blocking Broadcast failed!");
  }
}

template<class CC, class T>
void testGather(CC& cc, const T& data, const T& init){
  if(Dune::Span<T>::dynamic_size)
    return; // Can't gather dynamic objects
  { // blocking
    if(cc.rank() == 0){
      std::vector<T> vec(cc.size(), init);
      cc.gather(data, vec, 0);
      for(int i = 0; i < cc.size(); i++)
        if(vec[i] != data)
          DUNE_THROW(Dune::Exception, "Blocking gather failed!");
    }else{
      T b(init);
      cc.gather(data, b, 0);
    }
    // gatherv
    std::vector<T> buf(2, data);
    if(cc.rank() == 0){
      std::vector<T> vec(2*cc.size(), init);
      std::vector<int> recvlen(cc.size(), 2*Dune::Span<const T>(data).size());
      std::vector<int> displ(cc.size(), 0);
      for(int i = 1; i < cc.size(); i++)
        displ[i] = 2*i*Dune::Span<const T>(data).size();
      cc.gatherv(buf, vec, Dune::Span<std::vector<int>>(recvlen),
                 Dune::Span<std::vector<int>>(displ), 0);
      for(int i = 0; i < 2*cc.size(); i++)
        if(vec[i] != data)
          DUNE_THROW(Dune::Exception, "Blocking gatherv failed!");
    }else{
      T b(init);
      cc.gatherv(buf, b, {nullptr, 0}, {nullptr,0}, 0);
    }
  }
  { // non-blocking
    if(cc.rank() == 0){
      std::vector<T> vec(cc.size(), init);
      auto f = cc.igather(data, std::move(vec), 0);
      vec = f.get();
      for(int i = 0; i < cc.size(); i++)
        if(vec[i] != data)
          DUNE_THROW(Dune::Exception, "Non-blocking gather failed!");
    }else{
      auto f = cc.igather(data, init, 0);
      f.wait();
    }
  }
}

template<class CC, class T>
void testScatter(CC& cc, const T& data, const T& init){
  if(Dune::Span<T>::dynamic_size)
    return; // Can't scatter dynamic objects
  { // blocking
    T buf(init);
      std::vector<T> vec(cc.size(), data);
    if(cc.rank() == 0){
      cc.scatter(vec, buf, 0);
    }else{
      cc.scatter(vec, buf, 0);
    }
    if(buf != data)
      DUNE_THROW(Dune::Exception, "Blocking scatter failed!");
  }
  { // Scatterv
    std::array<T, 2> buf = {{init, init}};
    if(cc.rank() == 0){
      std::vector<T> vec(2*cc.size(), data);
      std::vector<int> sendlen(cc.size(), 2*Dune::Span<const T>(data).size());
      std::vector<int> displ(cc.size(), 0);
      for(int i = 1; i < cc.size(); i++)
        displ[i] = 2*i*Dune::Span<const T>(data).size();
      cc.scatterv(vec, Dune::Span<std::vector<int>>(sendlen),
                  Dune::Span<std::vector<int>>(displ), buf, 0);
    }else{
      cc.scatterv(init, {nullptr, 0},
                  {nullptr, 0}, buf, 0);
    }
    if(buf[0] != data || buf[1] != data)
      DUNE_THROW(Dune::Exception, "Blocking scatterv failed!");
  }
  { // non-blocking
    T buf(init);
    if(Dune::Span<T>::dynamic_size)
      Dune::Span<T>(buf).resize(Dune::Span<const T>(data).size());
    Dune::Future<T> f;
    if(cc.rank() == 0){
      std::vector<T> vec(cc.size(), data);
      f = cc.iscatter(vec, std::move(buf), 0);
      f.wait();
    }else{
      f = cc.iscatter(init, std::move(buf), 0);
    }
    if(f.get() != data)
      DUNE_THROW(Dune::Exception, "Non-blocking scatter failed!");
  }
}

template<class CC, class T>
void testAllgather(CC& cc, const T& data, const T& init){
  if(Dune::Span<T>::dynamic_size)
    return; // Can't scatter dynamic objects
  { //blocking
    std::vector<T> vec(cc.size(), init);
    cc.allgather(data, vec);
    for(int i = 0; i < cc.size(); i++){
      if(vec[i] != data)
        DUNE_THROW(Dune::Exception, "Blocking allgather failed!");
    }
  }
  { // allgatherv
    std::vector<T> vec(2*cc.size()*Dune::Span<const T>(data).size(), init);
    std::vector<int> recvlen(cc.size(), 2);
    std::vector<int> displ(cc.size(), 0);
    for(int i = 1; i < cc.size(); i++)
      displ[i] = i*2*Dune::Span<const T>(data).size();
    std::array<T, 2> buf = {{data, data}};
    cc.allgatherv(buf, vec, Dune::Span<std::vector<int>>(recvlen),
                  Dune::Span<std::vector<int>>(displ));
    for(size_t i = 0; i < vec.size(); i++){
      if(vec[i] != data)
        DUNE_THROW(Dune::Exception, "Blocking allgather failed!");
    }
  }
  { // non-blocking
    std::vector<T> vec(cc.size()*Dune::Span<const T>(data).size(), init);
    auto f = cc.iallgather(data, std::move(vec));
    vec = f.get();
    for(int i = 0; i < cc.size(); i++){
      if(vec[i] != data)
        DUNE_THROW(Dune::Exception, "Blocking allgather failed!");
    }
  }
}

template<class CC, class T>
void testAllreduce(CC& cc, const T& data, const T& init){
  typedef typename Dune::Span<T>::type integral_type;
  { // blocking in-place
    T buf(data);
    cc.template allreduce<Dune::Min<integral_type>>(buf);
    if(buf != data)
      DUNE_THROW(Dune::Exception, "Blocking allreduce (in-place) failed!");
  }
  { // blocking
    T buf(init);
    if(Dune::Span<T>::dynamic_size)
      Dune::Span<T>(buf).resize(Dune::Span<const T>(data).size());
    cc.template allreduce<Dune::Min<integral_type>>(data, buf);
    if(buf != data)
      DUNE_THROW(Dune::Exception, "Blocking allreduce failed!");
  }
  { // non-blocking in-place
    T buf(data);
    auto f = cc.template iallreduce<Dune::Max<integral_type>>(std::move(buf));
    if(f.get() != data)
      DUNE_THROW(Dune::Exception, "Non-blocking allreduce (in-place) failed!");
  }
  { // non-blocking
    T buf(init);
    if(Dune::Span<T>::dynamic_size)
      Dune::Span<T>(buf).resize(Dune::Span<const T>(data).size());
    auto f = cc.template iallreduce<Dune::Max<integral_type>>(data, std::move(buf));
    if(f.get() != data)
      DUNE_THROW(Dune::Exception, "Non-blocking allreduce failed!");
  }
}

template<>
void testAllreduce<decltype(Dune::MPIHelper::getCollectiveCommunication()),
                   std::complex<double>>
(decltype(Dune::MPIHelper::getCollectiveCommunication())&,
 const std::complex<double>&,
 const std::complex<double>&)
{
  return; // Disable function for std::complex
}

template<>
void testAllreduce<decltype(Dune::MPIHelper::getCollectiveCommunication()),
                   Dune::MPIPack<Dune::MPIHelper::MPICommunicator>>
(decltype(Dune::MPIHelper::getCollectiveCommunication())&,
 const Dune::MPIPack<Dune::MPIHelper::MPICommunicator>&,
 const Dune::MPIPack<Dune::MPIHelper::MPICommunicator>&)
{
  return; // Disable function for MPIPack
}

template<class CC, class T>
void testScan(CC& cc, const T& data, const T& init){
  typedef typename Dune::Span<T>::type integral_type;
  { // blocking scan
    T buf(init);
    if(Dune::Span<T>::dynamic_size)
      Dune::Span<T>(buf).resize(Dune::Span<const T>(data).size());
    cc.template scan<Dune::Min<integral_type>>(data, buf);
    if(buf != data)
      DUNE_THROW(Dune::Exception, "Blocking scan failed!");
  }
  { // blocking exscan
    T buf(init);
    if(Dune::Span<T>::dynamic_size)
      Dune::Span<T>(buf).resize(Dune::Span<const T>(data).size());
    cc.template exscan<Dune::Min<integral_type>>(data, buf);
    if(cc.rank() !=  0 && buf != data)
      DUNE_THROW(Dune::Exception, "Blocking exscan failed!");
  }
  { // non-blocking scan
    T buf(init);
    if(Dune::Span<T>::dynamic_size)
      Dune::Span<T>(buf).resize(Dune::Span<const T>(data).size());
    auto f = cc.template iscan<Dune::Max<integral_type>>(data, std::move(buf));
    if(f.get() != data)
      DUNE_THROW(Dune::Exception, "Non-blocking scan failed!");
  }
  { // non-blocking
    T buf(init);
    if(Dune::Span<T>::dynamic_size)
      Dune::Span<T>(buf).resize(Dune::Span<const T>(data).size());
    auto f = cc.template iexscan<Dune::Max<integral_type>>(data, std::move(buf));
    if(f.get() != data && cc.rank() != 0)
      DUNE_THROW(Dune::Exception, "Non-blocking exscan failed!");
  }
}

template<>
void testScan<decltype(Dune::MPIHelper::getCollectiveCommunication()),
                   std::complex<double>>
(decltype(Dune::MPIHelper::getCollectiveCommunication())&,
 const std::complex<double>&,
 const std::complex<double>&)
{
  return; // Disable function for std::complex
}

template<>
void testScan<decltype(Dune::MPIHelper::getCollectiveCommunication()),
                   Dune::MPIPack<Dune::MPIHelper::MPICommunicator>>
(decltype(Dune::MPIHelper::getCollectiveCommunication())&,
 const Dune::MPIPack<Dune::MPIHelper::MPICommunicator>&,
 const Dune::MPIPack<Dune::MPIHelper::MPICommunicator>&)
{
  return; // Disable function for MPIPack
}

template<class CC, class T>
void testAll(CC& cc, const T& data, const T& init = T{}){
  testBarrier(cc);
  testBroadcast(cc, data, init);
  testGather(cc, data, init);
  testScatter(cc, data, init);
  testAllgather(cc, data, init);
  testAllreduce(cc, data, init);
  testScan(cc, data, init);
}

int main(int argc, char** argv)
{
    int rank = -1;
  try{
    auto& helper = Dune::MPIHelper::instance(argc, argv);
    Dune::MPIHelper::MPICommunicator world = helper.getCommunicator();
    rank = world.rank();
    std::cout << "Rank " << rank << std::endl;
    auto cc = helper.getCollectiveCommunication();
    testAll(cc, 42); // int
    testAll(cc, 42.0f); // float
    testAll(cc, 42.0); // double
    testAll(cc, std::complex<double>(5, 7)); // std::complex
    testAll(cc, std::array<double, 2>{{42.0, 666.6}}); // std::array
    std::vector<std::array<double, 2>> arr_vec{{{42.9, 666.6}}, {{48149.0, 73388.0}}};
    testAll(cc, arr_vec);
    Dune::MPIPack<decltype(world)> pack(world);
    pack << 12 << 42.0 << std::string("Hello world!");
    Dune::MPIPack<decltype(world)> init(world);
    testAll(cc, pack, init);
  }catch(Dune::Exception& e)
  {
    std::cout << rank << ":\tcaught an exception!" << std::endl
              << e.what() << std::endl;
    return 1;
  }
  return 0;
}
