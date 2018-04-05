// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

/**
 * @file
 * @brief Implements resource management for MPI communicators
 * @author Nils-Arne Dreier
 * @ingroup ParallelCommunication
 */

#ifndef DUNE_COMMON_PARALLEL_MANAGEDMPICOMM_HH
#define DUNE_COMMON_PARALLEL_MANAGEDMPICOMM_HH

#include <memory>
#if HAVE_MPI
#include <mpi.h>
#if defined(OPEN_MPI) && DUNE_ENABLE_ULFM
#include <mpi-ext.h>
#endif
#include <dune/common/deprecated.hh>
#include <dune/common/stdstreams.hh>

#include "mpiexceptions.hh"
#include "mpifuture.hh"

namespace Dune
{

  /**! @brief wraps the MPI_Comm class and adds resource management
   * s.t.  MPI_Comm_free is automatically called, when no reference to
   * the communicator exists.
   *
   * Objects of this class are usable as MPI_Comm object, i.e. the
   * assignment operator does *not* duplicate the communicator. For
   * duplication the member function `dup()` is provided.
   *
   * @ingroup ParallelCommunication
  */
  class ManagedMPIComm
  {
  private:
    /* private constructor to wrap an existing MPI_Comm the shared_ptr
       should be equipped with an appropriate deleter.
    */
    explicit ManagedMPIComm(const std::shared_ptr<MPI_Comm>& pMPI_Comm)
      : comm_(pMPI_Comm)
    {
    }

  protected:
    // deleter function to be used with shared_ptr
    static void freeComm(MPI_Comm* c)
    {
      if (c != nullptr &&
          *c != MPI_COMM_WORLD &&
          *c != MPI_COMM_SELF &&
          *c != MPI_COMM_NULL)
      {
        int wasFinalized = 0;
        MPI_Finalized(&wasFinalized);
        if (!wasFinalized){
          int res = MPI_Comm_free(c);
          if(res == MPI_SUCCESS)
            *c = MPI_COMM_NULL;
          else
            dverb << "Cannot free MPI_Comm (" << *c << ")" << std::endl;
        }
      }
    }

    std::shared_ptr<MPI_Comm> comm_;
  public:
    /** @brief Default constructor. This resulting object connot be
     * used for communication. But another ManagedMPIComm object can
     * be assigned to this object. Internally it represents the
     * MPI_COMM_NULL communicator.
    */
    ManagedMPIComm()
      : ManagedMPIComm(std::make_shared<MPI_Comm>(MPI_COMM_NULL))
    {}

    /** @brief Wraps an native MPI_Comm into this class, but does not
        obtain resource management!
     */
    DUNE_DEPRECATED
    ManagedMPIComm(MPI_Comm c)
      : ManagedMPIComm(std::make_shared<MPI_Comm>(c))
    {}

    // Export Future types
    template<class T = void>
    using FutureType = MPIFuture<T>;

    /** @brief Returns a communicator containing all processes.
     */
    static ManagedMPIComm comm_world(){
      static ManagedMPIComm mmc_world(std::make_shared<MPI_Comm>(MPI_COMM_WORLD));
      return mmc_world;
    }

    /** @brief Returns a communicator containing only the local
     *  process.
     */
    static ManagedMPIComm comm_self(){
      static ManagedMPIComm mmc_self(std::make_shared<MPI_Comm>(MPI_COMM_SELF));
      return mmc_self;
    }

    /** @brief Convert to an MPI_Comm object. For use with native MPI
     *  Calls.
     */
    operator MPI_Comm () const{
      return *comm_;
    }

    /** @brief Returns the number of this process.
     */
    int rank() const{
      int rank;
      dune_mpi_call(MPI_Comm_rank, *comm_, &rank);
      return rank;
    }

    /** @brief Returns the size of the communicator.
     */
    int size() const
    {
      int size;
      dune_mpi_call(MPI_Comm_size, *comm_, &size);
      return size;
    }

    /** @brief See MPI_Comm_dup.
     *
     * @throw MPIError
     */
    ManagedMPIComm dup() const
    {
      std::shared_ptr<MPI_Comm> duplicate(new MPI_Comm(), freeComm);
      ManagedMPIComm mmc_duplicate(duplicate);
      dune_mpi_call(MPI_Comm_dup, *comm_, mmc_duplicate.comm_.get());
      return mmc_duplicate;
    }

    /** @brief See MPI_Comm_split.
     *
     * @throw MPIError
     */
    ManagedMPIComm split(int color, int key) const
    {
      std::shared_ptr<MPI_Comm> split(new MPI_Comm(), freeComm);
      ManagedMPIComm mmc_split(split);
      dune_mpi_call(MPI_Comm_split, *comm_, color, key,
                    mmc_split.comm_.get());
      return mmc_split;
    }

    /** @brief Replaces the internal MPI_Comm communicator object with
     *  a duplication of it. This might be useful to ensure that no
     *  pending requests are present.
     *
     * @throw MPIError
    */
    void renew() {
      MPI_Comm new_comm;
      dune_mpi_call(MPI_Comm_dup, *comm_, &new_comm);
      freeComm(comm_.get());
      *comm_ = new_comm;
    }

    /** @brief Returns true if this object stores a reasonable
     * Communicator (different from MPI_COMM_NULL).
     */
    operator bool() const
    {
      return comm_ && *comm_ != MPI_COMM_NULL;
    }

    /** @brief Returns true if the MPI_Comm object ob both
     * communicators are the same.
     */
    bool operator == (const ManagedMPIComm& other) const
    {
      return other.comm_ == comm_;
    }

#if MPI_VERSION > 3 || DUNE_ENABLE_ULFM
    // @brief See MPI_Comm_revoke (ULFM proposal)
    void revoke() {
      dune_mpi_call(MPIX_Comm_revoke, *comm_);
    }

    bool agree(bool success) {
      int flag = success?1:0;
      dune_mpi_call(MPIX_Comm_agree, *comm_, &flag);
      return flag!=0;
    }

    // @brief See MPI_Comm_shrink (ULFM proposal)
    void shrink() {
      MPI_Comm new_comm;
      dune_mpi_call(MPIX_Comm_shrink, *comm_, &new_comm);
      freeComm(comm_.get());
      *comm_ = new_comm;
    }
#else
    void revoke() {
      dinfo << "The Communicator can't be revoked. revoke() is not implemented." << std::endl;
      dune_mpi_call(MPI_Barrier, *comm_);
    }

    bool agree(bool success) {
      dune_mpi_call(MPI_Allreduce, MPI_IN_PLACE, &success, 1, MPI_CXX_BOOL, MPI_LAND, *this);
      return success;
    }

    void shrink() {
      dinfo << "The Communicator can't be shrunk. shrink() is not implemented." << std::endl;
    }
#endif
  };  // class ManagedMPIComm
}

#endif  // HAVE_MPI

namespace Dune {

  template<class T>
  class PseudoFuture;

  /* define some type that definitely differs from MPI_Comm */
  struct No_Comm {
    template<class T>
    using FutureType = PseudoFuture<T>;
    template<class T>
    using RecvFutureType = PseudoFuture<T>;
    template<class T>
    using ProbeFutureType = PseudoFuture<T>;
    constexpr int rank() { return 0; }
    constexpr int size() { return 1; }
    void revoke() {}
    void shrink() {}
    bool agree(bool success) {return success;}
  };
}
#endif  // DUNE_MANAGEDMPICOMM
