// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

/**
 * @file
 * @brief Implements a MPIGuard which detects an error on a remote process
 * @author Christian Engwer
 * @ingroup ParallelCommunication
 */

#ifndef DUNE_COMMON_MPIGUARD_HH
#define DUNE_COMMON_MPIGUARD_HH

#include "mpihelper.hh"
#include "communication.hh"
#include "mpicommunication.hh"
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
    static GuardCommunicator * create(const Communication<C> & c);
#if HAVE_MPI
    inline
    static GuardCommunicator * create(const MPI_Comm & c);
#endif
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
    // specialization for Communication
    template <class T>
    struct GenericGuardCommunicator< Communication<T> >
      : public GuardCommunicator
    {
      const Communication<T> comm;
      GenericGuardCommunicator(const Communication<T> & c) :
        comm(c) {}
      int rank() override { return comm.rank(); };
      int size() override { return comm.size(); };
      int sum(int i) override { return comm.sum(i); }
    };

#if HAVE_MPI
    // specialization for MPI_Comm
    template <>
    struct GenericGuardCommunicator<MPI_Comm>
      : public GenericGuardCommunicator< Communication<MPI_Comm> >
    {
      GenericGuardCommunicator(const MPI_Comm & c) :
        GenericGuardCommunicator< Communication<MPI_Comm> >(
          Communication<MPI_Comm>(c)) {}
    };
#endif
  }   // anonymous namespace

  template<class C>
  GuardCommunicator * GuardCommunicator::create(const Communication<C> & comm)
  {
    return new GenericGuardCommunicator< Communication<C> >(comm);
  }

#if HAVE_MPI
  GuardCommunicator * GuardCommunicator::create(const MPI_Comm & comm)
  {
    return new GenericGuardCommunicator< Communication<MPI_Comm> >(comm);
  }
#endif

#endif

  /*! @brief This exception is thrown if the MPIGuard detects an error on a remote process
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
     - Communication
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
              MPIHelper::getCommunication())),
      active_(active)
    {}

    /*! @brief create an MPIGuard operating on the Communicator of a special Dune::MPIHelper m

       @param m a reference to an MPIHelper
       @param active should the MPIGuard be active upon creation?
     */
    MPIGuard (MPIHelper & m, bool active=true) :
      comm_(GuardCommunicator::create(
              m.getCommunication())),
      active_(active)
    {}

    /*! @brief create an MPIGuard operating on an arbitrary communicator.

       Supported types for the communication object are:
       - MPIHelper
       - Communication
       - MPI_Comm

       @param comm reference to a communication object
       @param active should the MPIGuard be active upon creation?
     */
    template <class C>
    MPIGuard (const C & comm, bool active=true) :
      comm_(GuardCommunicator::create(comm)),
      active_(active)
    {}

#if HAVE_MPI
     MPIGuard (const MPI_Comm & comm, bool active=true) :
      comm_(GuardCommunicator::create(comm)),
      active_(active)
    {}
#endif

    /*! @brief destroy the guard and check for undetected exceptions
     */
    ~MPIGuard()
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
                   << result << " remote error(s)");
      }
    }
  };

}

#endif // DUNE_COMMON_MPIGUARD_HH
