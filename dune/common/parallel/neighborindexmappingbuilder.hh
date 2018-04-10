// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

/*!
  \file
  \brief Utility class for building neighbor - index information.
  \author Nils-Arne Dreier
  \ingroup ParallelCommunication
*/

#ifndef DUNE_COMMON_PARALLEL_NEIGHBORINDEXMAPPINGBUILDER_HH
#define DUNE_COMMON_PARALLEL_NEIGHBORINDEXMAPPINGBUILDER_HH

#include <map>

#include "pointtopointcommunication.hh"
#include "ringcommunication.hh"

namespace Dune {

  template<class Comm>
  class NeighborIndexMappingBuilder {

    PointToPointCommunication<Comm> ptpc_;
    RingCommunication<Comm> rc_;
    const int me_;
    const int procs_;

  public:
    NeighborIndexMappingBuilder(const Comm& c = MPIHelper::getCommunicator())
      : ptpc_(c)
      , rc_(c)
      , me_(c.rank())
      , procs_(c.size())
    {}

    // Generic IndexGraph construction
    // GI Global indices container
    // global_indices must be sorted
    // returns a map which associates all neighboring ranks a vector
    // of global indices in the order in which it is passed in at the
    // sender side
    template<typename GI>
    std::map<int, GI> buildNeighborIndexMapping(const GI& my_indices, int tag = 73388){
      std::map<int, GI> map;
      std::function<void(const GI&, int)> operation = [&map, &my_indices, this](const GI& remote_indices, int rank){
        if(rank != me_){
          GI buf;
          std::set_intersection(my_indices.begin(), my_indices.end(),
                                remote_indices.begin(), remote_indices.end(),
                                std::back_inserter(buf));
          if(buf.size() > 0)
            map[rank] = std::move(buf);
        }
      };
      rc_.applyRing(my_indices, operation, tag);
      return map;
    }

    // Generic IndexGraph construction using that neighbors already known
    // GI Global indices container
    // global_indices must be sorted
    // returns a map which associates all neighboring ranks a vector
    // of global indices in the order in which it is passed in at the
    // sender side
    template<typename GI>
    std::map<int, std::vector<typename GI::value_type>>
    buildNeighborIndexMapping(const GI& my_indices,
                              const std::initializer_list<int>& neighbors,
                              int tag = 314159){
      std::map<int, std::vector<typename GI::value_type>> map;
      // send my indices to all neighbors
      typedef typename Comm::template FutureType<> VoidFuture;
      std::vector<VoidFuture> sendRequests;
      sendRequests.reserve(neighbors.size());
      for(int n : neighbors) {
        sendRequests.push_back(ptpc_.isend(my_indices, n, tag));
      }
      // receive
      GI remote_indices;
      for(size_t i = 0; i < neighbors.size(); i++){
        auto s = ptpc_.mprobe(MPI_ANY_SOURCE, tag);
        int rank = s.source();
        s.recv(remote_indices);
        std::set_intersection(my_indices.begin(), my_indices.end(),
                              remote_indices.begin(), remote_indices.end(),
                              std::back_inserter(map[rank]));
      }
      return map;
    }
  };
}

#endif
