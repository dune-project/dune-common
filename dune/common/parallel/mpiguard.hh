// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

/**
 * @file
 * @brief Implements a MPIGuard which detects an error on a remote process
 * @author Christian Engwer
 * @ingroup ParallelCommunication
 */

#ifndef DUNE_COMMON_MPIGUARD_HH
#define DUNE_COMMON_MPIGUARD_HH

#include <vector>

#include "mpihelper.hh"
#include "collectivecommunication.hh"
#include "mpicollectivecommunication.hh"
#include <dune/common/exceptions.hh>

namespace Dune
{

#ifndef DOXYGEN

  /*
     Interface class for the communication needed by MPIGuard
   */
  struct GuardCommunicator
  {
    // cleanup
    virtual ~GuardCommunicator() {};
    // all the communication methods we need
    virtual int rank() = 0;
    virtual int size() = 0;
    virtual int sum(int i) = 0;
    // create a new GuardCommunicator pointer
    template <class C>
    static GuardCommunicator * create(const C & c);
  };

  namespace {
    /*
       templated implementation of different communication classes
     */
    // the default class will always fail, due to the missing implementation of "sum"
    template <class Imp>
    struct GenericGuardCommunicator
      : public GuardCommunicator
    {};
    // specialization for CollectiveCommunication
    template <class T>
    struct GenericGuardCommunicator< CollectiveCommunication<T> >
      : public GuardCommunicator
    {
      CollectiveCommunication<T> comm;
      GenericGuardCommunicator(const CollectiveCommunication<T> & c) :
        comm(c) {}
      int rank() override { return comm.rank(); };
      int size() override { return comm.size(); };
#if HAVE_ULFM_REVOKE
      int sum(int i) override {
        if(i != 0){
          comm.revoke();
        }
        int flag = i==0?1:0;
        comm.agree(flag);
        if(!flag){
          comm.shrink();
          return comm.sum(i);
        }else{
          return 0;
        }
      }
#else
      int sum(int i) override { return comm.sum(i); }
#endif
    };

#if HAVE_MPI
    // specialization for MPI_Comm
    template <>
    struct GenericGuardCommunicator<MPI_Comm>
      : public GenericGuardCommunicator< CollectiveCommunication<MPI_Comm> >
    {
      GenericGuardCommunicator(const MPI_Comm & c) :
        GenericGuardCommunicator< CollectiveCommunication<MPI_Comm> >(
          CollectiveCommunication<MPI_Comm>(c)) {}
    };
#endif
  }   // anonymous namespace

  template<class C>
  GuardCommunicator * GuardCommunicator::create(const C & comm)
  {
    return new GenericGuardCommunicator<C>(comm);
  }
#endif

  /*! @brief This exception is thrown if the MPIGuard detects an error on a remote process
      @ingroup ParallelCommunication
   */
  class MPIGuardError : public ParallelError {
    bool failed_;
  public:
    MPIGuardError(bool failed) :
      ParallelError(),
      failed_(failed)
    {}

    virtual bool failed() const{
      return failed_;
    }
  };

  /*! @brief detects a thrown exception and communicates to all other processes
      @ingroup ParallelCommunication

     @code
     {
       MPIGuard guard(...);

       do_something();

       // tell the guard that you successfully passed a critical operation
       guard.finalize();
       // reactivate the guard for the next critical operation
       guard.reactivate();

       int result = do_something_else();

       // tell the guard the result of your operation
       guard.finalize(result == success);
     }
     @endcode

     You create a MPIGuard object. If an exception is risen on a
     process the MPIGuard detects the exception, because the finalize
     method was not called.  When reaching the finalize call all
     other processes are informed that an error occurred and the
     MPIGuard throws an exception of type MPIGuardError.

     @note You can initialize the MPIGuard from different types of communication objects:
     - MPIHelper
     - CollectiveCommunication
     - MPI_Comm
   */
  class MPIGuard
  {
    GuardCommunicator * comm_;
    bool active_;

    // we don't want to copy this class
    MPIGuard (const MPIGuard &);

  public:
    /*! @brief create an MPIGuard operating on the Communicator of the global Dune::MPIHelper

       @param active should the MPIGuard be active upon creation?
     */
    MPIGuard (bool active=true) :
      comm_(GuardCommunicator::create(
              MPIHelper::getCollectiveCommunication())),
      active_(active)
    {}

    /*! @brief create an MPIGuard operating on the Communicator of a special Dune::MPIHelper m

       @param m a reference to an MPIHelper
       @param active should the MPIGuard be active upon creation?
     */
    MPIGuard (MPIHelper & m, bool active=true) :
      comm_(GuardCommunicator::create(
              m.getCollectiveCommunication())),
      active_(active)
    {}

    /*! @brief create an MPIGuard operating on an arbitrary communicator.

       Supported types for the communication object are:
       - MPIHelper
       - CollectiveCommunication
       - MPI_Comm

       @param comm reference to a communication object
       @param active should the MPIGuard be active upon creation?
     */
    template <class C>
    MPIGuard (const C & comm, bool active=true) :
      comm_(GuardCommunicator::create(comm)),
      active_(active)
    {}

    /*! @brief destroy the guard and check for undetected exceptions
     */
    ~MPIGuard() noexcept(false)
    {
      if (active_)
      {
        active_ = false;
        finalize(false);
      }
      delete comm_;
    }

    /*! @brief reactivate the guard.

       If the guard is still active finalize(true) is called first.
     */
    void reactivate() {
      if (active_ == true)
        finalize();
      active_ = true;
    }

    /*! @brief stop the guard.

       If no success parameter is passed, the guard assumes that
       everything worked as planned.  All errors are communicated
       and an exception of type MPIGuardError is thrown if an error
       (or exception) occurred on any of the processors in the
       communicator.

       @param success inform the guard about possible errors
     */
    void finalize(bool success = true)
    {
      int result = success ? 0 : 1;
      bool was_active = active_;
      active_ = false;
      result = comm_->sum(result);
      if (result>0 && was_active)
      {
        DUNE_THROW(MPIGuardError, "Terminating process "
                   << comm_->rank() << " due to "
                   << result << " remote error(s)", !success);
      }
    }

    /*
      Communicates the failed ranks. This function is collective. Each
      exception is considered as failed unless it is dynamic_castable
      to `MPIGuardException` and the `failed()` method returns false
      or, if ULFM_REVOKE is available, it is dynamic_castable to
      MPIError and the error class is MPIX_ERR_REVOKED.
     */
    template<class CC = std::decay_t<decltype(MPIHelper::getCollectiveCommunication())>>
    static std::vector<int> getFailedRanks(Exception& e, CC cc = MPIHelper::getCollectiveCommunication()){
      bool failed = true; // no MPIGuardException => local error
      MPIGuardError* mge = dynamic_cast<MPIGuardError*>(&e);
      if(mge != nullptr){
        failed = mge->failed(); // MPIGuardException might be sound
      }
#if HAVE_ULFM_REVOKE
      MPIError* me = dynamic_cast<MPIError*>(&e);
      if(me != nullptr){
        failed = me->errorClass() != MPIX_ERR_REVOKED;
      }
#endif
      // assign index to failed ranks
      int err_index = failed?1:0;
      cc.template scan<std::plus<int>>(&err_index, 1);
      int noErrors = err_index;
      // broadcast the number of failed ranks over the eintire communicator
      cc.broadcast(&noErrors, 1, cc.size()-1);
      // fill vector locally
      std::vector<int> ranks(noErrors, 0);
      if(failed)
        ranks[err_index-1] = cc.rank();
      // sum up all data, only one rank will contribute to coefficient
      cc.sum(ranks.data(), ranks.size());
      return ranks;
    }
  };

}

#endif // DUNE_COMMON_MPIGUARD_HH
