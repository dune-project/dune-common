// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

/**
 * @file
 * @brief Low-level wrapper for MPI-IO
 * @author Nils-Arne Dreier
 * @ingroup ParallelCommunication
 */

#ifndef DUNE_COMMON_PARALLEL_MPIFILE_HH
#define DUNE_COMMON_PARALLEL_MPIFILE_HH

#include <dune/common/parallel/mpiexceptions.hh>

#if MPI_VERSION >= 3

#include <mpi.h>

namespace Dune
{
  template<typename Comm>
  class MPIFile
  {

    Comm comm_;
    std::shared_ptr<MPI_File> file_ptr_;

    static void freeFile(MPI_File* f)
    {
      if(*f != MPI_FILE_NULL)
        dune_mpi_call(MPI_File_close, f);
    }

  public:
    template<class T = void>
    using FutureType = typename Comm::template FutureType<T>;
    template<class T = void>
    using RecvFutureType = typename Comm::template RecvFutureType<T>;

    MPIFile(const Comm& c, const char* filename,
            int amode = MPI_MODE_RDWR | MPI_MODE_CREATE,
            MPI_Info info = MPI_INFO_NULL)
      : comm_(c)
      , file_ptr_(new MPI_File, freeFile)
    {
      dune_mpi_call(MPI_File_open, comm_, filename, amode, info,
                    file_ptr_.get());
    }

    // enable default copy constructor
    MPIFile(const MPIFile&) = default;

    ~MPIFile() noexcept(false) // freeFile might throw
    {
    }

    void set_size(MPI_Offset s)
    {
      dune_mpi_call(MPI_File_set_size, *file_ptr_, s);
    }

    MPI_Offset get_size()
    {
      MPI_Offset s;
      dune_mpi_call(MPI_File_get_size, *file_ptr_, &s);
      return s;
    }

    void preallocate(MPI_Offset s)
    {
      dune_mpi_call(MPI_File_preallocate, *file_ptr_, s);
    }

    void set_info(MPI_Info i)
    {
      dune_mpi_call(MPI_File_set_info, *file_ptr_, i);
    }

    MPI_Info get_info()
    {
      MPI_Info i;
      dune_mpi_call(MPI_File_get_info, *file_ptr_, &i);
      return i;
    }

    int get_amode()
    {
      int amode;
      dune_mpi_call(MPI_File_get_amode, *file_ptr_, &amode);
      return amode;
    }

    void sync()
    {
      dune_mpi_call(MPI_File_sync, *file_ptr_);
    }

    void set_atomicity(bool flag)
    {
      dune_mpi_call(MPI_File_set_atomicity, *file_ptr_, flag);
    }

    bool get_atomicity() const
    {
      int flag;
      dune_mpi_call(MPI_File_get_atomicity, *file_ptr_, &flag);
      return flag;
    }

    /*!
      \param whence:
      MPI_SEEK_SET : the pointer is set to offset
      MPI_SEEK_CUR : the pointer is set to the current pointer position plus offset
      MPI_SEEK_END : the pointer is set to the end of file plus offset
    */

    enum Whence {
      set = MPI_SEEK_SET,
      cur = MPI_SEEK_CUR,
      end = MPI_SEEK_END
    };

    void seek(MPI_Offset offset, Whence whence = Whence::set)
    {
      dune_mpi_call(MPI_File_seek, *file_ptr_, offset, (int)whence);
    }

    MPI_Offset position(){
      MPI_Offset offset;
      dune_mpi_call(MPI_File_get_position, *file_ptr_, &offset);
      return offset;
    }

    MPI_Offset byte_offset(MPI_Offset offset){
      MPI_Offset disp;
      dune_mpi_call(MPI_File_get_byte_offset, *file_ptr_, offset, &disp);
      return disp;
    }

    template<typename T>
    RecvFutureType<std::decay_t<T>> iread_at(MPI_Offset offset, T&& data)
    {
      RecvFutureType<std::decay_t<T>> future(comm_, false, std::forward<T>(data));
      Span<std::decay_t<T>> span(future.buffer());
      dune_mpi_call(MPI_File_iread_at, *file_ptr_, offset,
                    span.ptr(), span.size(),
                    span.mpi_type(), &future.req_);
      return future;
    }

    template<typename T>
    FutureType<> iwrite_at(MPI_Offset offset, const T& data)
    {
      FutureType<> future(comm_, false);
      Span<std::decay_t<T>> span(data);
      dune_mpi_call(MPI_File_iwrite_at, *file_ptr_, offset,
                    span.ptr(), span.size(),
                    span.mpi_type(), &future.req_);
      return future;
    }

    template<typename T>
    RecvFutureType<std::decay_t<T>> iread(T&& data)
    {
      RecvFutureType<std::decay_t<T>> future(comm_, false, std::forward<T>(data));
      Span<std::decay_t<T>> span(future.buffer());
      dune_mpi_call(MPI_File_iread, *file_ptr_,
                    span.ptr(), span.size(),
                    span.mpi_type(),
                    &future.req_);
      return future;
    }

    template<typename T>
    FutureType<> iwrite(const T& data)
    {
      FutureType<> future(comm_, false);
      Span<std::decay_t<T>> span(data);
      dune_mpi_call(MPI_File_iwrite, *file_ptr_,
                    span.ptr(), span.size(),
                    span.mpi_type(),
                    &future.req_);
      return future;
    }

#if MPI_VERSION > 3 || (MPI_VERSION==3 && MPI_SUBVERSION >=1)
    template<typename T>
    RecvFutureType<std::decay_t<T>> iread_at_all(MPI_Offset offset, T&& data)
    {
      RecvFutureType<std::decay_t<T>> future(comm_, true, std::forward<T>(data));
      Span<std::decay_t<T>> span (future.buffer());
      dune_mpi_call(MPI_File_iread_at_all, *file_ptr_, offset,
                    span.ptr(), span.size(),
                    span.mpi_type(), &future.req_);
      return future;
    }

    template<typename T>
    FutureType<> iwrite_at_all(MPI_Offset offset, const T& data)
    {
      FutureType<> future(comm_, false);
      Span<std::decay_t<T>> span(data);
      dune_mpi_call(MPI_File_iwrite_at_all, *file_ptr_, offset,
                    span.ptr(), span.size(),
                    span.mpi_type(), &future.req_);
      return future;
    }

    template<typename T>
    RecvFutureType<std::decay_t<T>> iread_all(T&& data)
    {
      RecvFutureType<std::decay_t<T>> future(comm_, false, std::forward<T>(data));
      Span<std::decay_t<T>> span(future.buffer());
      dune_mpi_call(MPI_File_iread_all, *file_ptr_,
                    span.ptr(), span.size(),
                    span.mpi_type(),
                    &future.req_);
      return future;
    }

    template<typename T>
    FutureType<> iwrite_all(const T& data)
    {
      FutureType<> future(comm_, false);
      Span<std::decay_t<T>> span(data);
      dune_mpi_call(MPI_File_iwrite_all, *file_ptr_,
                    span.ptr(), span.size(),
                    span.mpi_type(),
                    &future.req_);
      return future;
    }
#endif

    // Stream interface to be used with Archive:
    void write(char* data, size_t size)
    {
      write(Span<char*>(data, size)).wait();
    }

    void read(char* data, size_t size)
    {
      read(Span<char*>(data, size)).wait();
    }
  };  // class MPIFile

  inline void delete_file(const char* filename, MPI_Info info = MPI_INFO_NULL)
  {
    dune_mpi_call(MPI_File_delete, filename, info);
  }
}  // namespace Dune

#endif

#endif
