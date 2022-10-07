// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

/**
 * @brief Benchmark for measure the possible overlap of computation
 * and communication at MPI collective communications.
 *
 * This benchmark is inspired by the sandia micro benchmark:
 * W. Lawry, C. Wilson, A. Maccabe, R. Brightwell. COMB: A Portable Benchmark
 * Suite for Assessing MPI Overlap. In Proceedings of the IEEE International
 * Conference on Cluster Computing (CLUSTER 2002), p. 472, 2002.
 * http://www.cs.sandia.gov/smb/overhead.html
 *
 * The following communication times are measured:
 * Blocking:                       Blocking call. E.g. MPI_Allreduce
 * Nonblocking_wait (NB_Wait):     Nonblocking (e.g. MPI_Iallreduce) call
 *                                 directly followed by MPI_Wait.
 * Nonblocking_sleep (NB_Sleep):   Nonblocking call followed by a busy wait
 *                                 until the work time has passed. Then
 *                                 MPI_Wait.
 * Nonblocking_active (NB_active): Nonblocking call followed by a basy wait
 *                                 where in every iteration MPI_Test is
 *                                 called until the work time has passed.
 *                                 The MPI_wait.
 *
 * The overhead is computed as the time for the Nonblocking call plus
 * the time for MPI_Wait. The iteration time is the time for the whole
 * communication. The available part of the communication
 * time(avail(%)) is computed as 1-(overhead/base_t), where base_t is
 * the time for calling the method with wait time = 0. The overhead is
 * determined by increasing the work time successive until it is the
 * dominant factor in the iteration time. Then the overhead is
 * computed as iter_t-work_t.
 *
 * Usage: mpirun ./mpi_collective_benchmark [options]
 *
 * options:
 * -method: default: allreduce.
 *                   possible methods: allreduce, barrier,
 *                   broadcast, gather, allgather, scatter
 * -iterations: default: 10000. Number of iterations for
 *              measure the time for one communication
 * -allMethods: default:0. If 1 iterates over all available methods
 * -startSize: default: n, where n is the size of MPI_COMM_WORLD. runs
 *             the benchmark for different communicator sizes, starting with
 *             startSize. After every run the size is doubled. Finally one run is
 *             made for the whole communicator.
 * -verbose: default: 0. If 1 prints intermediate information while determining
 *           the overhead.
 * -threshold: default: 2. The threshold when the work time is the dominant
 *             factor in the iteration time. (Similar to the threshold in the
 *             sandia benchmark)
 * -nohdr: default: 0. Suppress output of the header.
 *
 * options can be set either in the options.ini file or can be pass at
 * the command-line (-key value).
 *
 * To get a good 'available' value for the NB_sleep communication, some
 * MPI implementation need to spawn an extra thread. With MPICH you
 * can activate this by setting the environment variable
 * MPI_ASYNC_PROGRESS to 1, with IntelMPI the variable is called
 * I_MPI_ASYNC_PROGRESS.
 * (https://software.intel.com/en-us/mpi-developer-reference-linux-asynchronous-progress-control)
 */


#include <config.h>

#include <iostream>
#include <iomanip>
#include <thread>

#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/timer.hh>
#include <dune/common/parametertree.hh>
#include <dune/common/parametertreeparser.hh>

Dune::ParameterTree options;
std::vector<std::string> all_methods = {"allreduce",
                         "barrier",
                         "broadcast",
                         "gather",
                         "allgather",
                         "scatter"};

template<class CC>
void communicate(CC& cc){
  auto method = options.get("method", "allreduce");
  std::vector<int> data(1, 42);
  if(method == "allreduce"){
    cc.template allreduce<std::plus<int>>(data);
    return;
  }
  if(method == "barrier"){
    cc.barrier();
    return;
  }
  if(method == "broadcast"){
    cc.broadcast(data.data(), data.size(), 0);
    return;
  }
  if(method == "gather"){
    std::vector<int> recv_data(cc.size(), 0);
    cc.gather(data.data(), recv_data.data(), 1, 0);
    return;
  }
  if(method == "allgather"){
    std::vector<int> recv_data(cc.size(), 0);
    cc.allgather(data.data(), 1, recv_data.data());
    return;
  }
  if(method == "scatter"){
    std::vector<int> send_data(cc.size(), 42);
    cc.scatter(send_data.data(), data.data(), 1, 0);
    return;
  }
  DUNE_THROW(Dune::Exception, "Unknown method");
}

template<class CC>
Dune::Future<void> startCommunication(CC& cc){
  auto method = options.get("method", "allreduce");
  if(method == "allreduce"){
    return cc.template iallreduce<std::plus<char>>(42);
  }
  if(method == "barrier"){
    return cc.ibarrier();
  }
  if(method == "broadcast"){
    return cc.ibroadcast(42, 0);
  }
  if(method == "gather"){
    return cc.igather(42, std::vector<int>(cc.size()), 0);
  }
  if(method == "allgather"){
    return cc.iallgather(42, std::vector<int>(cc.size()));
  }
  if(method == "scatter"){
    return cc.iscatter(std::vector<int>(cc.size(), 42), 0, 0);
  }
  DUNE_THROW(Dune::Exception, "Unknown method");
}

template<class CC>
double runBlocking(CC& cc){
  std::vector<char> answer(1, 42);
  int iterations = options.get("iterations", 1000);
  Dune::Timer watch;
  for(int i = 0; i < iterations; i++){
    cc.barrier();
    watch.start();
    communicate(cc);
    watch.stop();
  }
  return cc.sum(watch.elapsed())/iterations/cc.size();
}

template<class CC>
double runNonblockingWait(CC& cc){
  std::vector<char> answer(1, 42);
  Dune::Timer watch;
  int iterations = options.get("iterations", 1000);
  for(int i = 0; i < iterations; i++){
    cc.barrier();
    watch.start();
    auto f = startCommunication(cc);
    f.wait();
    watch.stop();
  }
  return cc.sum(watch.elapsed())/iterations/cc.size();
}

std::tuple<double, double> runNonblockingSleep(decltype(Dune::MPIHelper::getCommunication())& cc, std::chrono::duration<double> wait_time){
  std::vector<char> answer(1, 42);
  Dune::Timer watch, watch_work;
  int iterations = options.get("iterations", 1000);
  for(int i = 0; i < iterations; i++){
    cc.barrier();
    watch.start();
    auto f = startCommunication(cc);
    watch_work.start();
    auto start_time = std::chrono::high_resolution_clock::now();
    while(std::chrono::high_resolution_clock::now()-start_time < wait_time);
    watch_work.stop();
    f.wait();
    watch.stop();
  }
  return std::tuple<double, double>(cc.sum(watch.stop())/iterations/cc.size(),
                                    cc.sum(watch_work.stop())/iterations/cc.size());
}

std::tuple<double, double> runNonblockingActive(decltype(Dune::MPIHelper::getCommunication())& cc, std::chrono::duration<double> wait_time){
  std::vector<char> answer(1, 42);
  int iterations = options.get("iterations", 1000);
  Dune::Timer watch, watch_work;
  for(int i = 0; i < iterations; i++){
    cc.barrier();
    watch.start();
    auto f = startCommunication(cc);
    watch_work.start();
    auto start_time = std::chrono::high_resolution_clock::now();
    while(std::chrono::high_resolution_clock::now()-start_time < wait_time)
      f.ready();
    watch_work.stop();
    f.wait();
    watch.stop();
  }
  // return the time spend in communication methods
  return std::tuple<double, double>(cc.sum(watch.stop())/iterations/cc.size(),
                                    cc.sum(watch_work.stop())/iterations/cc.size());
}

/* Increases the work until it is the dominant factor in the iteration
   time. Returns the base time and how much of it is available for
   computations(%). It is computed with the formula 1-(overhead/base_t).
 */
std::tuple<double, double> determineOverlap(std::function<std::tuple<double, double>(std::chrono::duration<double>)> fun)
{
  double base_t = 0;
  std::tie(base_t, std::ignore) = fun(std::chrono::duration<double>(0));
  if(options.get("verbose", 0))
    std::cout << std::endl << std::endl << std::setw(12) << "base_t:" << base_t << std::endl;
  double iter_t = 0;
  double work_t = 0;
  int i = 1;
  double iter_t_threshold = options.get("threshold", 2.0);
  for(double work = 0.25*base_t; iter_t < iter_t_threshold*base_t; work *= 2, i++){
    std::tie(iter_t, work_t) = fun(std::chrono::duration<double>(work));
    if(options.get("verbose", 0))
      std::cout << i << std::setw(12) << " iter_t:" << std::setw(12) << iter_t
                << std::setw(12) << " work_t:" << std::setw(12) << work_t << std::endl;
  }
  double overhead = iter_t-work_t;
  double avail = 1.0-overhead/base_t;
  if(options.get("verbose", 0))
    std::cout << std::setw(12) << " ovhd:" << std::setw(12) << overhead
              << std::setw(12) << " available:"  << std::setw(12) << avail << std::endl;
  return std::tuple<double, double>(base_t, avail);
}

void printHeader(){
  if(options.get("nohdr", 0) == 0){
    std::cout << "Method: " << options.get("method", "allreduce") << std::endl;
    std::cout << std::scientific;
    std::cout << std::setw(10) << "commsize"
              << std::setw(12) << "iterations"
              << std::setw(16) << "Blocking"
              << std::setw(16) << "NB_wait"
              << std::setw(16) << "NB_sleep"
              << std::setw(12) << "avail(%)"
              << std::setw(16) << "NB_active"
              << std::setw(12) << "avail(%)"
              << std::endl;
  }
}

void run(int s){
  auto comm_world = Dune::MPIHelper::getCommunication();
  Dune::MPIHelper::MPICommunicator comm;
  #if HAVE_MPI
  MPI_Comm_split(comm_world, comm_world.rank() < s, comm_world.rank(), &comm);
  #endif
  if(comm_world.rank() < s){
    Dune::Communication<Dune::MPIHelper::MPICommunicator> cc(comm);
    std::cout << std::setw(10) << cc.size()
              << std::setw(12) << options.get("iterations", 1000) << std::flush;

    double blocking_t = runBlocking(cc);
    std::cout << std::setw(16) << blocking_t << std::flush;

    double nb_wait_t = runNonblockingWait(cc);
    std::cout << std::setw(16) << nb_wait_t << std::flush;

    using namespace std::placeholders;
    auto nb_sleep = std::bind(runNonblockingSleep, std::ref(cc), _1);
    double nb_sleep_t, nb_sleep_avail;
    std::tie(nb_sleep_t, nb_sleep_avail) = determineOverlap(nb_sleep);
    std::cout << std::setw(16) << nb_sleep_t
              << std::setw(12) << std::fixed << std::setprecision(2) << 100*nb_sleep_avail
              << std::scientific << std::setprecision(6) << std::flush;

    auto nb_active = std::bind(runNonblockingActive, cc, _1);
    double nb_active_t, nb_active_avail;
    std::tie(nb_active_t, nb_active_avail) = determineOverlap(nb_active);
    std::cout << std::setw(16) << nb_active_t
              << std::setw(12) << std::fixed << std::setprecision(2) << 100*nb_active_avail
              << std::scientific << std::setprecision(6) << std::endl;
  }
}

int main(int argc, char** argv){
  Dune::MPIHelper& mpihelper = Dune::MPIHelper::instance(argc, argv);

  // disable output on almost all ranks
  if(mpihelper.rank() != 0)
    std::cout.setstate(std::ios_base::failbit);
  // parse options
  Dune::ParameterTreeParser::readINITree("options.ini", options);
  Dune::ParameterTreeParser::readOptions(argc, argv, options);

  std::vector<std::string> methods = {options.get("method", "allreduce")};
  if(options.get("allMethods", 0) == 1)
    methods = std::vector<std::string>(all_methods);
  for(std::string method : methods){
    options["method"] = method;
    std::cout << std::left << std::scientific;
    printHeader();
    int s = options.get("startSize", mpihelper.size());
    while(s < mpihelper.size()){
      run(s);
      s *= 2;
    }
    run(mpihelper.size());
  }
  return 0;
}
