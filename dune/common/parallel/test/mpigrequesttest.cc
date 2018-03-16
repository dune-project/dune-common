// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <future>
#include <chrono>
#include <thread>
#include <mutex>

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/parallel/mpigrequest.hh>

int main(int argc, char** argv)
{
  auto& helper = Dune::MPIHelper::instance(argc, argv);
  auto world = helper.getCommunicator();
#if HAVE_MPI

  std::shared_ptr<std::mutex> mutex = std::make_shared<std::mutex>();
  mutex->lock();
  auto work = [=](Dune::MPIStatus& s){
    mutex->lock();
    mutex->unlock();
  };
  auto cancel = [=](bool complete){
    mutex->unlock();
  };

  Dune::MPIGRequest<decltype(work), decltype(cancel)> r(std::move(work), std::move(cancel));

  std::thread canceler([=]() mutable {
      std::this_thread::sleep_for(std::chrono::seconds(2));
      std::cout << "cancelling..." << std::endl;
      MPI_Cancel(r);
    });

  MPI_Status s;
  int err = MPI_Wait(r, &s);
  std::cout << "err = " << err << std::endl;
  canceler.join();
#endif
  return 0;
}
