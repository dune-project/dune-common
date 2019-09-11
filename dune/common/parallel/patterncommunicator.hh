// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_PARALLEL_PATTERNCOMMUNICATOR_HH
#define DUNE_COMMON_PARALLEL_PATTERNCOMMUNICATOR_HH

#include <map>
#include <unordered_map>
#include <functional>
#include <dune/common/shared_ptr.hh>
#include <dune/common/parallel/mpipack.hh>

namespace Dune {

  template<class Pattern, class Communication, class Buffer>
  class PatternCommunicator{

    typedef typename Pattern::remote_type remote_type;
    typedef typename Pattern::index_type index_type;

    typedef std::unordered_map<int, Buffer> tag_to_buffer_map;
    typedef std::unordered_map<int, Future<Buffer&>> tag_to_future_map;

    std::map<remote_type, tag_to_buffer_map> sendBuffer;
    std::map<remote_type, tag_to_buffer_map> recvBuffer;

    std::map<remote_type, tag_to_future_map> sendFutures;
    std::map<remote_type, tag_to_future_map> recvFutures;
  public:
    PatternCommunicator(const Pattern& pattern,
                        Communication comm,
                        size_t fixedSizePerIndex = 0)
      : PatternCommunicator(stackobject_to_shared_ptr(pattern)
                            , comm
                            , fixedSizePerIndex)
    {}

    PatternCommunicator(std::shared_ptr<const Pattern> pattern,
                        Communication comm,
                        size_t fixedSizePerIndex = 0)
      : pattern_(pattern)
      , comm_(comm)
      , fixedSizePerIndex_(fixedSizePerIndex)
    {}

    void exchange(std::function<void(Buffer&, const index_type&)> gather,
                  std::function<void(Buffer&, const index_type&)> scatter,
                  int tag = 4711){
      // setup recv futures
      if(fixedSizePerIndex_){
        for(const auto& pair : pattern_->recvPattern()){
          const remote_type& remote = pair.first;
          auto& future = recvFutures[remote][tag];
          auto& buffer = recvBuffer[remote][tag];
          buffer.reserve(pair.second.size()*fixedSizePerIndex_);
          future = comm_.template irecv<Buffer&>(buffer, remote, tag);
        }
      }
      // setup send futures
      for(const auto& pair : pattern_->sendPattern()){
        const remote_type& remote = pair.first;
        auto& future = sendFutures[remote][tag];
        auto& buffer = sendBuffer[remote][tag];
        if(future.valid())
          future.wait();
        buffer.reserve(pair.second.size()*fixedSizePerIndex_);
        buffer.seek(0);
        for(const index_type& idx: pair.second){
          gather(buffer, idx);
        }
        future = comm_.template isend<Buffer&>(buffer, remote, tag);
      }
      if(fixedSizePerIndex_){
        // finish recv futures:
        for(const auto& pair : pattern_->recvPattern()){
          const remote_type& remote = pair.first;
          auto& future = recvFutures[remote][tag];
          auto& buffer = future.get();
          buffer.seek(0);
          for(const index_type& idx : pair.second){
            scatter(buffer, idx);
          }
        }
      }else{
        for(const auto& pair : pattern_->recvPattern()){
          const remote_type& remote = pair.first;
          auto& buffer = recvBuffer[remote][tag];
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

    const Pattern& communicationPattern() const{
      return *pattern_;
    }

  protected:
    std::shared_ptr<const Pattern> pattern_;
    Communication comm_;
    const size_t fixedSizePerIndex_;
  };

#if HAVE_MPI
  template<class Pattern>
  using MPIPatternCommunicator = PatternCommunicator<Pattern, Communication<MPI_Comm>, MPIPack>;
#endif
}

#endif
