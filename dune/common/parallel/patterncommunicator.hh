// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_PARALLEL_PATTERNCOMMUNICATOR_HH
#define DUNE_COMMON_PARALLEL_PATTERNCOMMUNICATOR_HH

#include <map>
#include <unordered_map>
#include <functional>
#include <dune/common/parallel/mpipack.hh>

namespace Dune {

  template<class Pattern, class Communication, class Buffer>
  class PatternCommunicator{

    typedef typename Pattern::remote_type remote_type;
    typedef typename Pattern::index_type index_type;

    typedef std::unordered_map<int, Buffer> tag_to_buffer_map;
    typedef std::unordered_map<int, Future<Buffer&>> tag_to_future_map;

    std::map<remote_type, tag_to_buffer_map> send_buffer;
    std::map<remote_type, tag_to_buffer_map> recv_buffer;

    std::map<remote_type, tag_to_future_map> send_futures;
    std::map<remote_type, tag_to_future_map> recv_futures;
  public:
    PatternCommunicator(const Pattern& pattern,
                        Communication comm,
                        size_t fixed_size_per_index = 0)
      : pattern_(pattern)
      , comm_(comm)
      , fixed_size_per_index_(fixed_size_per_index)
    {}

    void exchange(std::function<void(Buffer&, const index_type&)> gather,
                  std::function<void(Buffer&, const index_type&)> scatter,
                  int tag = 4711){
      // setup recv futures
      if(fixed_size_per_index_){
        for(const auto& pair : pattern_.recv_pattern()){
          const remote_type& remote = pair.first;
          auto& future = recv_futures[remote][tag];
          auto& buffer = recv_buffer[remote][tag];
          buffer.reserve(pair.second.size()*fixed_size_per_index_);
          future = comm_.template irecv<Buffer&>(buffer, remote, tag);
        }
      }
      // setup send futures
      for(const auto& pair : pattern_.send_pattern()){
        const remote_type& remote = pair.first;
        auto& future = send_futures[remote][tag];
        auto& buffer = send_buffer[remote][tag];
        if(future.valid())
          future.wait();
        buffer.reserve(pair.second.size()*fixed_size_per_index_);
        buffer.seek(0);
        for(const index_type& idx: pair.second){
          gather(buffer, idx);
        }
        future = comm_.template isend<Buffer&>(buffer, remote, tag);
      }
      if(fixed_size_per_index_){
        // finish recv futures:
        for(const auto& pair : pattern_.recv_pattern()){
          const remote_type& remote = pair.first;
          auto& future = recv_futures[remote][tag];
          auto& buffer = future.get();
          buffer.seek(0);
          for(const index_type& idx : pair.second){
            scatter(buffer, idx);
          }
        }
      }else{
        for(const auto& pair : pattern_.recv_pattern()){
          const remote_type& remote = pair.first;
          auto& buffer = recv_buffer[remote][tag];
          comm_.rrecv(buffer, remote, tag);
          buffer.seek(0);
          for(const index_type& idx : pair.second){
            scatter(buffer, idx);
          }
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
        tag);
    }

    const Pattern& communication_pattern() const{
      return pattern_;
    }

  protected:
    const Pattern& pattern_;
    Communication comm_;
    const size_t fixed_size_per_index_;
  };

#if HAVE_MPI
  template<class Pattern>
  using MPIPatternCommunicator = PatternCommunicator<Pattern, Communication<MPI_Comm>, MPIPack>;
#endif
}

#endif
