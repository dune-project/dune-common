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

#include "mpihelper.hh"
#include "collectivecommunication.hh"
#include "mpicollectivecommunication.hh"
#include <dune/common/exceptions.hh>

namespace Dune
{
  /*! @brief This exception is thrown if the MPIGuard detects an error
      on a remote process
      @ingroup ParallelCommunication
   */
  class MPIGuardError : public ParallelError {};

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

     @note It is not necessary to call finalize before leaving the
     scope, since the MPIGuard will also detect thrown exceptions by
     std::uncaught_exceptions.
   */
  class MPIGuard
  {
    MPIHelper::MPICommunicator comm;
    bool active_;
#if DUNE_HAVE_CXX_UNCAUGHT_EXCEPTIONS
    int uncaught_exceptions_;
#endif

    // we don't want to copy this class
    MPIGuard (const MPIGuard &) = delete;

  public:
    /*! @brief create an MPIGuard operating on the Communicator of the global Dune::MPIHelper

       @param active should the MPIGuard be active upon creation?
     */
    MPIGuard (bool active=true) :
      MPIGuard(MPIHelper::getCommunicator(), active)
    {}

    /*! @brief create an MPIGuard operating on the Communicator of a special Dune::MPIHelper m

       @param m a reference to an MPIHelper
       @param active should the MPIGuard be active upon creation?
     */
    MPIGuard (MPIHelper & m, bool active=true) :
      MPIGuard(m.getCommunicator(), active)
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
      comm(comm),
      active_(active)
#if DUNE_HAVE_CXX_UNCAUGHT_EXCEPTIONS
      , uncaught_exceptions_(std::uncaught_exceptions())
#endif
    {
      // wait until all process are in safe area
      CollectiveCommunication<C> cc(this->comm);
      cc.barrier();
    }

    /*! @brief destroy the guard and check for undetected exceptions
     */
    ~MPIGuard() noexcept(false)
    {
      dverb << "MPIGuard::~MPIGuard()" << std::endl;
      if (active_)
      {
#if DUNE_HAVE_CXX_UNCAUGHT_EXCEPTIONS
        if(uncaught_exceptions_ < std::uncaught_exceptions())
#else
        if(std::uncaught_exception())
#endif
        {
          try{
            finalize(false);
          } catch(...) {}
        } else {
          finalize(true); // might throw
        }
      }
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
      active_ = false;
      if (!success){
        comm.revoke();
#if DUNE_HAVE_CXX_UNCAUGHT_EXCEPTIONS
        if(uncaught_exceptions_ >= std::uncaught_exceptions())
#else
        if(!std::uncaught_exception())
#endif
          DUNE_THROW(MPIGuardError, "Terminating process "
                     << comm.rank());
      }else{
        CollectiveCommunication<decltype(comm)> cc(comm);
        cc.barrier();
      }
    }
  };

}

#endif // DUNE_COMMON_MPIGUARD_HH
