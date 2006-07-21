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
  /**
   * @file
   * @brief This file contains helpers for dealing with MPI.
   *
   * Basically there are two helpers available:
   * <dl>
   *   <dt>FakeMPIHelper</dt>
   *   <dd>A class adhering to the interface of MPIHelper
   *       that does not need MPI at all. This can be used
   *       to create a sequential program even if MPI is
   *       used to compile it.
   *   </dd>
   *   <dt>MPIHelper</dt>
   *   <dd>A real MPI helper. When the singleton
   *       gets instantiated MPI_Init will be
   *       called and before the program exits
   *       MPI_Finalize will be called.
   *   </dd>
   * </dl>
   *
   * Example of who to use these classes:
   *
   *   A program that is parallel if compiled with MPI
   *   and sequential otherwise:
   *   \code
   *   int main(int argc, char** argv){
   *      typedef Dune::MPIHelper MPIHelper;
   *      MPIHelper::instance(&argc, &argv);
   *      typename MPIHelper::MPICommunicator world =
   *        MPIHelper::getCommunicator();
   *      ...
   *   \endcode
   *
   * If one wants to have sequential program even if the code is
   * compiled with mpi then one simply has to exchange the typedef
   * with \code typedef Dune::MPIHelper FakeMPIHelper; \endcode.
   */
  /**
   * @brief A fake mpi helper.
   *
   * This helper can be used if no MPI is available
   * or one wants to run sequentially even if MPI is
   * available and used.
   */
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

    /**
     * @brief Get the default communicator.
     *
     * @return -1 As we are fake.
     */
    static MPICommunicator getCommunicator()
    {
      return -1;
    }

    /**
     * @brief Get the singleton instance of the helper.
     *
     * This method has to be called with the same arguments
     * that the main method of the program was called:
     * \code
     * int main(int argc, char** argv){
     *   MPIHelper::instance(&argc, &argv);
     *   // program code comes here
     *   ...
     * }
     * \endcode
     * @param argc The number of arguments provided to main.
     * @param argv The arguments provided to main.
     */
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
  /**
   * @brief A real mpi helper.
   *
   * This helper should be used for parallel programs.
   */
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

    /**
     * @brief Get the default communicator.
     *
     * @return MPI_COMM_WORLD
     */
    static MPICommunicator getCommunicator(){
      return MPI_COMM_WORLD;
    }

    /**
     * @brief Get the singleton instance of the helper.
     *
     * This method has to be called with the same arguments
     * that the main method of the program was called:
     * \code
     * int main(int argc, char** argv){
     *   MPIHelper::instance(&argc, &argv);
     *   // program code comes here
     *   ...
     * }
     * \endcode
     * @param argc The number of arguments provided to main.
     * @param argv The arguments provided to main.
     */
    static MPIHelper& instance(int& argc, char**& argv)
    {
      if(instance_.get() == 0)
        instance_.set(new MPIHelper(argc, argv));
      return *instance_.get();
    }

  private:
    MPIHelper(int& argc, char**& argv)
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
  /** @brief If no MPI is available FakeMPIHelper becomes the MPIHelper */
#define MPIHelper FakeMPIHelper

#endif

} // end namespace Dune
#endif
