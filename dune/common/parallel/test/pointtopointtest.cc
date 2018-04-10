// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/dynvector.hh>
#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpipack.hh>
#include <dune/common/parallel/pointtopointcommunication.hh>

template<class P2PC, class T>
void test_blocking(P2PC& p2pc, const T& data, const T& init){
  if(p2pc.rank() == 0){
    p2pc.send(data, 1, 42);
    p2pc.send(data, 1, 42);
    p2pc.send(data, 1, 42);
    p2pc.send(data, 1, 42);
    p2pc.send(data, 1, 42);
  }
  if(p2pc.rank() == 1){
    // recv
    {
      T recv_buffer = init;
      p2pc.recv(recv_buffer, 0, 42, true);
      if(recv_buffer != data)
        DUNE_THROW(Dune::Exception, "1: received data does not match");
    }
    // probe
    {
      Dune::MPIStatus s = p2pc.probe(0, 42);
      T recv_buf = init;
      p2pc.recv(recv_buf, 0, 42, true);
      if(s.source() != 0 || recv_buf != data)
        DUNE_THROW(Dune::Exception, "2: received data does not match");
    }
    // mprobe
    {
      auto s = p2pc.mprobe(0, 42);
      T recv_buf = init;
      s.recv(recv_buf);
      if(s.source() != 0 || recv_buf != data)
        DUNE_THROW(Dune::Exception, "3: received data does not match");
    }
    // iprobe
    {
      Dune::MPIStatus s;
      do{
        s = p2pc.iprobe(0, 42);
      }while(s.isEmpty());
      T recv_buf = init;
      p2pc.recv(recv_buf, 0, 42, true);
      if(s.source() != 0 || recv_buf != data)
        DUNE_THROW(Dune::Exception, "4: received data does not match");
    }
    // improbe
    {
      T recv_buf = init;
      do{
        auto s = p2pc.improbe(0, 42);
        if(s.hasMessage()){
          s.recv(recv_buf);
          if(s.source() != 0 || recv_buf != data)
            DUNE_THROW(Dune::Exception, "4: received data does not match");
          break;
        }
      }while(true);
    }
  }
}

template<class P2PC, class T>
void test_nonblocking(P2PC& p2pc, const T& data, const T& init){
  if(p2pc.rank() == 0){
    auto f = p2pc.isend(data, 1, 3);
    auto f2 = p2pc.isend(data, 1, 3);
    f2.wait();
    when_all(std::move(f), std::move(f2)).wait();
  }
  if(p2pc.rank() == 1){
    auto f = p2pc.irecv(init, 0, 3, true);
    T buf(init);
    Dune::Span<T>(buf).resize(Dune::Span<const T>(data).size());
    auto f2 = p2pc.irecv(std::move(buf), 0, 3, false);
    when_all(f, f2).wait();
    //f.wait();
    //f2.wait();
    auto recv = f.get();
    if(recv != data)
      DUNE_THROW(Dune::Exception, "5: received data does not match");
    if(f.status().source() != 0)
      DUNE_THROW(Dune::Exception, "5: received source does not match");
    if((size_t)f.status().count(Dune::Span<const T>::mpiType()) != Dune::Span<const T>(data).size())
      DUNE_THROW(Dune::Exception, "5: received count does not match");
    auto recv2 = f2.get();
    auto st = f2.status();
    if(recv2 != data || st.source() != 0 ||
       (size_t)st.count(Dune::Span<const T>::mpiType()) != Dune::Span<const T>(data).size())
      DUNE_THROW(Dune::Exception, "6: received data does not match");
  }
}

template<class P2PC, class T>
void testDataType(P2PC& p2pc, const T& data, const T& init = T{}){
  test_blocking(p2pc, data, init);
  test_nonblocking(p2pc, data, init);
}

int main(int argc, char** argv)
{
  int rank = -1;
  try{
    auto& helper = Dune::MPIHelper::instance(argc, argv);
    Dune::MPIHelper::MPICommunicator world = helper.getCommunicator();
    rank = world.rank();
    std::cout << "Rank " << rank << std::endl;
    auto p2pc = helper.getPointToPointCommunication();
    if(world.size() > 1){
      std::cout << "test int..." << std::endl;
      testDataType(p2pc, 42); // int
      std::cout << "test float..." << std::endl;
      testDataType(p2pc, 42.0f); // float
      std::cout << "test double..." << std::endl;
      testDataType(p2pc, 42.0); // double
      std::cout << "test std::complex..." << std::endl;
      testDataType(p2pc, std::complex<double>(5, 7)); // std::complex
      std::cout << "test std::array..." << std::endl;
      testDataType(p2pc, std::array<double, 2>{{42.0, 666.6}}); // std::array
      std::cout << "test std::vector..." << std::endl;
      std::vector<std::array<double, 2>> arr_vec{{{{42.9, 666.6}}, {{48149.0, 73388.0}}}};
      testDataType(p2pc, arr_vec);
      std::cout << "test MPIPack..." << std::endl;
      Dune::MPIPack<decltype(world)> pack(world);
      pack << 12 << 42.0 << std::string("Hello world!");
      Dune::MPIPack<decltype(world)> init(world);
      testDataType(p2pc, pack, init);
      std::cout << "test DynamicVector..." << std::endl;
      Dune::DynamicVector<std::complex<double>> dynvec {{{42, 3}, {7.33, 8.8}}};
      testDataType(p2pc, dynvec);
    }

  }catch(Dune::Exception& e)
  {
    std::cout << rank << ":\tcaught an exception!" << std::endl
              << e.what() << std::endl;
    return 1;
  }
  return 0;
}
