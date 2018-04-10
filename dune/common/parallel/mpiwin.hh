// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

/**
 * @file
 * @brief Low-level wrapper for MPI-IO
 * @author Nils-Arne Dreier
 * @ingroup ParallelCommunication
 */

#ifndef DUNE_COMMON_PARALLEL_MPIWIN_HH
#define DUNE_COMMON_PARALLEL_MPIWIN_HH

#include <dune/common/parallel/mpiexceptions.hh>

#if MPI_VERSION >= 3
#include <mpi.h>

namespace Dune
{
    template<class Comm>
    class MPIWin
    {
      template<class T = void>
      using FutureType = typename Comm::template FutureType<T>;

      Comm comm_;
      std::shared_ptr<MPI_Win> win_ptr_;

      static void freeWin(MPI_Win* w){
        if(*w != MPI_WIN_NULL)
          duneMPICall(MPI_Win_free, w);
      }

    public:
      template<class T>
      MPIWin(const Comm& c, T& data, int displ_unit = 1, MPI_Info info = MPI_INFO_NULL)
        : comm_(c)
        , win_ptr_(new MPI_Win, freeWin)
      {
        Span<T> span(data);
        int size = span.size()*sizeof(typename Span<T>::type);
        duneMPICall(MPI_Win_create, span.ptr(), size,
                      displ_unit, info, comm_, win_ptr_.get());
      }

      // enable default constructor
      MPIWin(const MPIWin&) = default;

      ~MPIWin() noexcept(false) // freeWin might throw
      {
      }

      template<class T>
      FutureType<> put(const T& data, int target_rank, int target_displ)
      {
        FutureType<> future(comm_, false);
        Span<std::decay_t<T>> span(data);
        duneMPICall(MPI_Rput, span.ptr(), span.size(),
                      span.mpiType(), target_rank, target_displ,
                      span.size(), span.type(),
                      *win_ptr_, &future.mpiRequest());
        return future;
      }

      template<class T>
      FutureType<std::decay_t<T>> get(int target_rank, int target_displ, T&& data)
      {
        FutureType<std::decay_t<T>> future(comm_, false, std::forward<T>(data));
        Span<std::decay_t<T>> span(future.buffer());
        duneMPICall(MPI_Rget, span.ptr(), span.size(),
                      span.mpiType(), target_rank, target_displ, span.size(),
                      span.mpiType(), *win_ptr_, &future.mpiRequest());
        return future;
      }

      template<class BinaryFunction, class T>
      FutureType<> accumulate(const T& data, int target_rank, int target_displ)
      {
        FutureType<> future(comm_, false);
        Span<T> span(data);
        duneMPICall(MPI_Raccumulate, span.ptr(), span.size(),
                      span.mpiType(), target_rank, target_displ,
                      span.size(), span.type(),
                      Generic_MPI_Op<typename decltype(span)::type, BinaryFunction>::get(),
                      *win_ptr_, &future.mpiRequest());
        return future;
      }

      template<class BinaryFunction, class T>
      FutureType<std::decay_t<T>> getAccumulate(const T& data, int target_rank,
                                             int target_displ, T&& result)
      {
        Span<std::decay_t<T>> span_data(data);
        FutureType<std::decay_t<T>> future(comm_, false, std::forward<T>(result));
        Span<std::decay_t<T>> span_result(future.buffer());
        duneMPICall(MPI_Rget_accumulate, span_data.ptr(),
                      span_data.size(), span_data.mpiType(),
                      span_result.ptr(), span_result.size(),
                      span_result.mpiType(), target_rank, target_displ,
                      span_data.size(),
                      span_data.mpiType(),
                      Generic_MPI_Op<typename decltype(span_data)::type, BinaryFunction>::get(),
                      *win_ptr_, &future.mpiRequest());
        return future;
      }

      void fence(int assert = 0){
        duneMPICall(MPI_Win_fence, assert, *win_ptr_);
      }

      void start(MPI_Group g, int assert = 0)
      {
        duneMPICall(MPI_Win_start, g, assert, *win_ptr_);
      }

      void complete()
      {
        duneMPICall(MPI_Win_complete, *win_ptr_);
      }

      void post(MPI_Group g, int assert = 0)
      {
        duneMPICall(MPI_Win_post, g, assert, *win_ptr_);
      }

      void wait()
      {
        duneMPICall(MPI_Win_wait, *win_ptr_);
      }

      bool test()
      {
        int flag = 0;
        duneMPICall(MPI_Win_test, *win_ptr_, &flag);
        return flag;
      }

      void lock(int lock_type, int rank, int assert = 0)
      {
        duneMPICall(MPI_Win_lock, lock_type, rank, assert, *win_ptr_);
      }

      void lockAll(int assert = 0)
      {
        duneMPICall(MPI_Win_lock_all, assert, *win_ptr_);
      }

      void unlock(int rank)
      {
        duneMPICall(MPI_Win_unlock, rank, *win_ptr_);
      }

      void unlockAll()
      {
        duneMPICall(MPI_Win_unlock_all, *win_ptr_);
      }

      void flush(int rank)
      {
        duneMPICall(MPI_Win_flush, rank, *win_ptr_);
      }

      void flushAll()
      {
        duneMPICall(MPI_Win_flush_all, *win_ptr_);
      }

      void flushLocal(int rank)
      {
        duneMPICall(MPI_Win_flush_local, rank, *win_ptr_);
      }

      void flushLocalAll()
      {
        duneMPICall(MPI_Win_flush_local_all, *win_ptr_);
      }

      void sync()
      {
        duneMPICall(MPI_Win_sync, *win_ptr_);
      }
    };
}  // namespace Dune

#endif

#endif
