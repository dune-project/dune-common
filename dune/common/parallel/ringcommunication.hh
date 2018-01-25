// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

/*!
  \file
  \brief Implements an generic ringcommunication
  \author Nils-Arne Dreier
  \ingroup ParallelCommunication
*/

#ifndef DUNE_COMMON_PARALLEL_RINGCOMMUNICATION_HH
#define DUNE_COMMON_PARALLEL_RINGCOMMUNICATION_HH

#include "pointtopointcommunication.hh"

namespace Dune{
  template<class Comm>
  class RingCommunication{
    PointToPointCommunication<Comm> ptpc_;
    const int rank_;
    const int size_;
    const int left_;
    const int right_;
  public:
    RingCommunication ( const Comm& c = Comm::comm_world())
      : ptpc_(c)
      , rank_(c.rank())
      , size_(c.size())
      , left_((rank_-1+size_)%size_)
      , right_((rank_+1)%size_)
    {}

    // Generic ring communication
    template<typename T>
    void applyRing(const T& data, std::function<void(const T&, int)> fun,
                   bool applySelf = true, int tag = 42424242)
    {
      T send_buffer(data);
      T recv_buffer;
      if(applySelf)
        fun(send_buffer, rank_);
      for(int i = 0; i < size_-1; i++){
        // use Span since we want to access the object during the communication
        auto send_future = ptpc_.isend(send_buffer, left_, tag);
        ptpc_.recv(recv_buffer, right_, tag);
        // apply the data (during the communication)
        fun(recv_buffer, (i+rank_)%size_);
        send_future.wait();
        std::swap(send_buffer, recv_buffer);
      }
    }

  };
}

#endif
