// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id: $
#ifndef DUNE_MPIHELPER
#define DUNE_MPIHELPER
#if HAVE_MPI
#include "mpi.h"
#endif

namespace Dune
{

  class FakeMPIHelper
  {
  public:
    /**
     * @brief A simple smart pointer responsible for creation
     * and deletion of the instance.
     */
    class InstancePointer
    {
    public:
      /** @brief Construct a null pointer. */
      InstancePointer() : pointer_(0)
      {}
      /** @brief Delete the instance we point to. */
      ~InstancePointer()
      {
        if(pointer_ != 0)
          delete pointer_;
      }
      /**
       * @brief Get a pointer to the instance.
       * @return The instance we store.
       */
      FakeMPIHelper* get()
      {
        return pointer_;
      }
      /**
       * @brief Set the pointer.
       * @paramter pointer A pointer to the instance.
       */
      void set(FakeMPIHelper* pointer)
      {
        if(pointer != 0) {
          delete pointer_;
          pointer_ = pointer;
        }
      }
    private:
      FakeMPIHelper* pointer_;
    };
  public:
    enum {
      /**
       * @brief Are we fake (i. e. pretend to have MPI support but are compiled
       * without.
       */
      isFake = true
    };


    /**
     * @brief The type of the mpi communicator.
     */
    typedef int MPICommunicator;

    static MPICommunicator getCommunicator()
    {
      return -1;
    }

    static FakeMPIHelper& instance(int argc, char** argv)
    {
      if(instance_.get() == 0)
        instance_.set(new FakeMPIHelper());
      return *instance_.get();
    }

  private:
    FakeMPIHelper()
    {}
    FakeMPIHelper(const FakeMPIHelper&);
    FakeMPIHelper& operator=(const FakeMPIHelper);

    static InstancePointer instance_;
  };

  FakeMPIHelper::InstancePointer FakeMPIHelper::instance_ = FakeMPIHelper::InstancePointer();

#ifdef HAVE_MPI

  class MPIHelper
  {
  public:
    /**
     * @brief A simple smart pointer responsible for creation
     * and deletion of the instance.
     */
    class InstancePointer
    {
    public:
      /** @brief Construct a null pointer. */
      InstancePointer() : pointer_(0)
      {}
      /** @brief Delete the instance we point to. */
      ~InstancePointer()
      {
        if(pointer_ != 0)
          delete pointer_;
      }
      /**
       * @brief Get a pointer to the instance.
       * @return The instance we store.
       */
      MPIHelper* get()
      {
        return pointer_;
      }
      /**
       * @brief Set the pointer.
       * @paramter pointer A pointer to the instance.
       */
      void set(MPIHelper* pointer)
      {
        if(pointer != 0) {
          delete pointer_;
          pointer_ = pointer;
        }
      }
    private:
      MPIHelper* pointer_;
    };
  public:
    enum {
      /**
       * @brief Are we fake (i. e. pretend to have MPI support but are compiled
       * without.
       */
      isFake = false
    };

    /**
     * @brief The type of the mpi communicator.
     */
    typedef MPI_Comm MPICommunicator;

    static MPICommunicator getCommunicator(){
      return MPI_COMM_WORLD;
    }

    static MPIHelper& instance(int argc, char** argv)
    {
      if(instance_.get() == 0)
        instance_.set(new MPIHelper(argc, argv));
      return *instance_.get();
    }

  private:
    MPIHelper(int argc, char** argv)
    {
      MPI_Init(&argc, &argv);
    }
    ~MPIHelper()
    {
      MPI_Finalize();
    }
    MPIHelper(const MPIHelper&);
    MPIHelper& operator=(const MPIHelper);

    static InstancePointer instance_;
  };

  MPIHelper::InstancePointer MPIHelper::instance_ = MPIHelper::InstancePointer();

#else
  // We do not have MPI therefore FakeMPIHelper
  // is the MPIHelper
#define MPIHelper FakeMPIHelper

#endif

} // end namespace Dune
#endif
