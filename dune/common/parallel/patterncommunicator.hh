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
    typedef Buffer buffer_type;

    typedef std::unordered_map<int, Buffer> tag_to_buffer_map;
    typedef std::unordered_map<int, Future<Buffer&>> tag_to_future_map;

    std::map<remote_type, tag_to_buffer_map> sendBuffers_;
    std::map<remote_type, tag_to_buffer_map> recvBuffers_;

    std::map<remote_type, tag_to_future_map> sendFutures_;
    std::map<remote_type, tag_to_future_map> recvFutures_;
  public:
    PatternCommunicator(const Pattern& pattern,
                        Communication comm)
      : PatternCommunicator(stackobject_to_shared_ptr(pattern)
                            , comm)
    {}

    PatternCommunicator(std::shared_ptr<const Pattern> pattern,
                        Communication comm)
      : pattern_(pattern)
      , comm_(comm)
    {}

    /*
      GATHERFUN and SCATTERFUN are functors with the following interface:
      void(buffer_type&, index_type)

      where the buffer can be accessed via `read` and `write` methods and the
      index can be implicitly casted to size_t and provide access to the local
      and remote attribute via `localAttribute()` and `remoteAttribute()`.
     */
    template<class GATHERFUN, class SCATTERFUN>
    void exchange(GATHERFUN gather,
                  SCATTERFUN scatter,
                  int tag = 4711){
      // setup send futures
      for(const auto& pair : *pattern_){
        const remote_type& remote = pair.first;
        auto& future = sendFutures_[remote][tag];
        auto& buffer = sendBuffers_[remote][tag];
        if(future.valid())
          future.wait();
        buffer.seek(0);
        for(const index_type& idx: pair.second){
          gather(buffer, idx);
        }
        future = comm_.template isend<Buffer&>(buffer, remote, tag);
      }
      for(const auto& pair : *pattern_){
        const remote_type& remote = pair.first;
        auto& buffer = recvBuffers_[remote][tag];
        comm_.rrecv(buffer, remote, tag);
        buffer.seek(0);
        for(const index_type& idx : pair.second){
          scatter(buffer, idx);
        }
      }
    }

    template<class V, class FromSet, class ToSet, class ReductionFunctor>
    void exchange(const V& source,
                  V& dest,
                  FromSet fromSet,
                  ToSet toSet,
                  const ReductionFunctor& reduction_functor,
                  int tag = 4712){
      exchange([&](auto& buf, const auto& idx){
                 if(fromSet.contains(idx.localAttribute()) &&
                    toSet.contains(idx.remoteAttribute()))
                   buf.write(source[idx]);
               },
        [&](auto& buf, const auto& idx){
          if(toSet.contains(idx.localAttribute()) &&
             fromSet.contains(idx.remoteAttribute())){
            auto temp = dest[idx];
            buf.read(temp);
            dest[idx] = reduction_functor(dest[idx], temp);
          }},
        tag);
    }

    const Pattern& communicationPattern() const{
      return *pattern_;
    }

  protected:
    std::shared_ptr<const Pattern> pattern_;
    Communication comm_;
  };

#if HAVE_MPI
  template<class Pattern>
  using MPIPatternCommunicator =
    PatternCommunicator<Pattern, Communication<MPI_Comm>, MPIPack>;
#endif
}

#endif
