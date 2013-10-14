// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id: $
#ifndef DUNE_MPIHELPER
#define DUNE_MPIHELPER

#include <cassert>
#include "collectivecommunication.hh"
#if HAVE_MPI
#include "mpi.h"
#include "mpicollectivecommunication.hh"
#endif

#include <dune/common/stdstreams.hh>
#include <dune/common/visibility.hh>

namespace Dune
{
  /**
   * @file
   * @brief Helpers for dealing with MPI.
   *
   * @ingroup ParallelCommunication
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
   *      MPIHelper::instance(argc, argv);
   *      typename MPIHelper::MPICommunicator world =
   *        MPIHelper::getCommunicator();
   *      ...
   *   \endcode
   *
   * If one wants to have sequential program even if the code is
   * compiled with mpi then one simply has to exchange the typedef
   * with \code typedef Dune::MPIHelper FakeMPIHelper; \endcode.
   *
   * For checking whether we really use MPI or just fake please use
   * MPIHelper::isFake (this is also possible at compile time!)
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
    enum {
      /**
       * @brief Are we fake (i.e. pretend to have MPI support but are compiled
       * without.)
       */
      isFake = true
    };

    /**
     * @brief The type of the mpi communicator.
     */
    typedef No_Comm MPICommunicator;

    /** \brief get the default communicator
     *
     *  Return a communicator to exchange data with all processes
     *
     *  \returns a fake communicator
     */
    DUNE_EXPORT static MPICommunicator getCommunicator ()
    {
      static MPICommunicator comm;
      return comm;
    }

    /** \brief get a local communicator
     *
     *  Returns a communicator to communicate with the local process only
     *
     *  \returns a fake communicator
     */
    static MPICommunicator getLocalCommunicator ()
    {
      return getCommunicator();
    }



    static CollectiveCommunication<MPICommunicator>
    getCollectiveCommunication()
    {
      return CollectiveCommunication<MPICommunicator>(getCommunicator());
    }

    /**
     * @brief Get the singleton instance of the helper.
     *
     * This method has to be called with the same arguments
     * that the main method of the program was called:
     * \code
     * int main(int argc, char** argv){
     *   MPIHelper::instance(argc, argv);
     *   // program code comes here
     *   ...
     * }
     * \endcode
     * @param argc The number of arguments provided to main.
     * @param argv The arguments provided to main.
     */
    DUNE_EXPORT static FakeMPIHelper& instance(int argc, char** argv)
    {
      // create singleton instance
      static FakeMPIHelper singleton;
      return singleton;
    }

    /**
     * @brief return rank of process, i.e. zero
     */
    int rank () const { return 0; }
    /**
     * @brief return rank of process, i.e. one
     */
    int size () const { return 1; }

  private:
    FakeMPIHelper() {}
    FakeMPIHelper(const FakeMPIHelper&);
    FakeMPIHelper& operator=(const FakeMPIHelper);
  };

#if HAVE_MPI
  /**
   * @brief A real mpi helper.
   * @ingroup ParallelCommunication
   *
   * This helper should be used for parallel programs.
   */
  class MPIHelper
  {
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

    /** \brief get the default communicator
     *
     *  Return a communicator to exchange data with all processes
     *
     *  \returns MPI_COMM_WORLD
     */
    static MPICommunicator getCommunicator ()
    {
      return MPI_COMM_WORLD;
    }

    /** \brief get a local communicator
     *
     *  Returns a communicator to exchange data with the local process only
     *
     *  \returns MPI_COMM_SELF
     */
    static MPICommunicator getLocalCommunicator ()
    {
      return MPI_COMM_SELF;
    }

    static CollectiveCommunication<MPICommunicator>
    getCollectiveCommunication()
    {
      return CollectiveCommunication<MPICommunicator>(getCommunicator());
    }
    /**
     * @brief Get the singleton instance of the helper.
     *
     * This method has to be called with the same arguments
     * that the main method of the program was called:
     * \code
     * int main(int argc, char** argv){
     *   MPIHelper::instance(argc, argv);
     *   // program code comes here
     *   ...
     * }
     * \endcode
     * @param argc The number of arguments provided to main.
     * @param argv The arguments provided to main.
     */
    DUNE_EXPORT static MPIHelper& instance(int& argc, char**& argv)
    {
      // create singleton instance
      static MPIHelper singleton (argc, argv);
      return singleton;
    }

    /**
     * @brief return rank of process
     */
    int rank () const { return rank_; }
    /**
     * @brief return number of processes
     */
    int size () const { return size_; }

  private:
    int rank_;
    int size_;
    void prevent_warning(int){}

    //! \brief calls MPI_Init with argc and argv as parameters
    MPIHelper(int& argc, char**& argv)
    {
#if MPI_2
      int wasInitialized = -1;
      MPI_Initialized( &wasInitialized );
      if(!wasInitialized)
#endif
      {
        rank_ = -1;
        size_ = -1;
        static int is_initialized = MPI_Init(&argc, &argv);
        prevent_warning(is_initialized);
      }

      MPI_Comm_rank(MPI_COMM_WORLD,&rank_);
      MPI_Comm_size(MPI_COMM_WORLD,&size_);

      assert( rank_ >= 0 );
      assert( size_ >= 1 );

      dverb << "Called  MPI_Init on p=" << rank_ << "!" << std::endl;
    }
    //! \brief calls MPI_Finalize
    ~MPIHelper()
    {
#ifdef MPI_2
      int wasFinalized = -1;
      MPI_Finalized( &wasFinalized );
      if(!wasFinalized) {
#endif
      MPI_Finalize();
      dverb << "Called MPI_Finalize on p=" << rank_ << "!" <<std::endl;
#ifdef MPI_2
    }

#endif
    }
    MPIHelper(const MPIHelper&);
    MPIHelper& operator=(const MPIHelper);
  };
#else
  // We do not have MPI therefore FakeMPIHelper
  // is the MPIHelper
  /**
   * @brief If no MPI is available FakeMPIHelper becomes the MPIHelper
   * @ingroup ParallelCommunication
   */
  typedef FakeMPIHelper MPIHelper;

#endif

} // end namespace Dune
#endif
