// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

/**
 * @file @brief Wrapper for MPI_Status and MPI_Message.
 *
 * @author Nils-Arne Dreier
 * @ingroup ParallelCommunication
 */

#ifndef DUNE_COMMON_PARALLEL_MPISTATUS_HH
#define DUNE_COMMON_PARALLEL_MPISTATUS_HH

#include "span.hh"

#if HAVE_MPI
#include <mpi.h>

#include "mpiexceptions.hh"

namespace Dune {
  class MPIStatus {

  protected:
    MPI_Status status_;
  public:
    MPIStatus(const MPI_Status& s) :
      status_(s) {}

    MPIStatus()
    {
      // set status to empty
      status_.MPI_TAG = MPI_ANY_TAG;
      status_.MPI_SOURCE = MPI_ANY_SOURCE;
      status_.MPI_ERROR = MPI_SUCCESS;
      duneMPICall(MPI_Status_set_cancelled, &status_, 0);
    }

    /** @brief Return the source of the message
     */
    int source() const{
      return status_.MPI_SOURCE;
    }

    /** @brief Return the size of the message
     */
    int count(const MPI_Datatype& datatype) const{
      int count = -1;
      duneMPICall(MPI_Get_count, &status_, datatype, &count);
      return count;
    }

    /** @brief Return the MPI error code of the message.
     */
    int error() const {
      return status_.MPI_ERROR;
    }

    /** @brief Set the MPI error code of the message.
     */
    void setError(int e) {
      status_.MPI_ERROR = e;
    }

    /** @brief Return the tag of the message.
     */
    int tag() const {
      return status_.MPI_TAG;
    }

    // used for native MPI calls
    operator MPI_Status* (){
      return &status_;
    }

    operator MPI_Status (){
      return status_;
    }

    /** @brief Checks whether this objects contains information.
     */
    bool isEmpty() const{
      return status_.MPI_TAG==MPI_ANY_TAG &&
        status_.MPI_SOURCE == MPI_ANY_SOURCE &&
        status_.MPI_ERROR == MPI_SUCCESS;
    }

    bool isCancelled() const{
      int flag = 0;
      duneMPICall(MPI_Test_cancelled, &status_, &flag);
      return flag;
    }

    void setCancelled(bool cancel = true){
      duneMPICall(MPI_Status_set_cancelled, &status_, cancel?1:0);
    }
  };

  template<class T>
  class MPIProbeFuture;

  class MPIMatchingStatus : public MPIStatus {
    template<class T>
    friend class MPIProbeFuture;
    template<class T>
    friend class BlackChannelProbeFuture;
    MPI_Message message_;

  public:
    MPIMatchingStatus(const MPI_Status& s,
                      const MPI_Message& m = MPI_MESSAGE_NULL) :
      MPIStatus(s),
      message_(m)
    {}

    MPIMatchingStatus() :
      MPIStatus(),
      message_(MPI_MESSAGE_NULL)
    {}

    /** @brief Receives the data of the messages matched with the
     * probe.
     */
    template<typename T>
    void recv(T& data){
      Span<T> span(data);
      if(Span<T>::dynamicSize)
        span.resize(count(span.mpiType()));
      duneMPICall(MPI_Mrecv, span.ptr(), span.size(),
                    span.mpiType(), &message_, &status_);
    }

    /** @brief Checks whether this objects has a matched message
     */
    bool hasMessage() const {
      return message_ != MPI_MESSAGE_NULL;
    }
  };
}
#endif
#endif
