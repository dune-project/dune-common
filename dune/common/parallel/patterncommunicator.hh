// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_PARALLEL_PATTERNCOMMUNICATOR_HH
#define DUNE_COMMON_PARALLEL_PATTERNCOMMUNICATOR_HH

#include <functional>
#include <dune/common/parallel/mpipack.hh>

namespace Dune {

  template<class Pattern, class Communication, class Buffer>
  class PatternCommunicator{

    typedef typename Pattern::remote_type remote_type;
    typedef typename Pattern::index_type index_type;
    std::map<remote_type, Buffer> send_buffer;
    std::map<remote_type, Buffer> recv_buffer;

    std::map<remote_type, Future<Buffer&>> send_futures;
    std::map<remote_type, Future<Buffer&>> recv_futures;
  public:
    PatternCommunicator(const Pattern& pattern,
                        Communication comm)
      : pattern_(pattern)
      , comm_(comm)
    {}

    void exchange(std::function<void(Buffer&, const index_type&)> gather,
                  std::function<void(Buffer&, const index_type&)> scatter,
                  std::size_t size_per_index = 1,
                  int tag = 4711){
      // setup recv futures
      for(const auto& pair : pattern_.recv_pattern()){
        const remote_type& remote = pair.first;
        recv_buffer[remote].reserve(pair.second.size()*size_per_index);
        recv_futures[remote] = comm_.template irecv<Buffer&>(recv_buffer[remote], remote, tag);
      }
      // setup send futures
      for(const auto& pair : pattern_.send_pattern()){
        const remote_type& remote = pair.first;
        if(send_futures[remote].valid())
          send_futures[remote].wait();
        send_buffer[remote].reserve(pair.second.size()*size_per_index);
        send_buffer[remote].seek(0);
        for(const index_type& idx: pair.second){
          gather(send_buffer[remote], idx);
        }
        send_futures[remote] = comm_.template isend<Buffer&>(send_buffer[remote], remote, tag);
      }
      // finish recv futures:
      for(const auto& pair : pattern_.recv_pattern()){
        const remote_type& remote = pair.first;
        auto& buffer = recv_futures[remote].get();
        recv_buffer[remote].seek(0);
        for(const index_type& idx : pair.second){
          scatter(buffer, idx);
        }
      }
    }

    template<class V, class ReductionFunctor>
    void exchange(const V& source,
                  V& dest,
                  const ReductionFunctor& reduction_functor,
                  int tag = 4712){
      exchange([&](auto& buf, const auto& idx){
                 buf.write(source[idx]);
               },
        [&](auto& buf, const auto& idx){
          auto temp = dest[idx];
          buf.read(temp);
          dest[idx] = reduction_functor(dest[idx], temp);
        },
        sizeof(typename V::value_type),
        tag);
    }

  protected:
    const Pattern& pattern_;
    Communication comm_;
  };

#if HAVE_MPI
  template<class Pattern>
  using MPIPatternCommunicator = PatternCommunicator<Pattern, Communication<MPI_Comm>, MPIPack>;
#endif
}

#endif