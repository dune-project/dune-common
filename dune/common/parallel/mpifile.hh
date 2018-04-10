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
        duneMPICall(MPI_File_close, f);
    }

  public:
    template<class T = void>
    using FutureType = typename Comm::template FutureType<T>;

    MPIFile(const Comm& c, const char* filename,
            int amode = MPI_MODE_RDWR | MPI_MODE_CREATE,
            MPI_Info info = MPI_INFO_NULL)
      : comm_(c)
      , file_ptr_(new MPI_File, freeFile)
    {
      duneMPICall(MPI_File_open, comm_, filename, amode, info,
                    file_ptr_.get());
    }

    // enable default copy constructor
    MPIFile(const MPIFile&) = default;

    ~MPIFile() noexcept(false) // freeFile might throw
    {
    }

    void setSize(MPI_Offset s)
    {
      duneMPICall(MPI_File_set_size, *file_ptr_, s);
    }

    MPI_Offset size()
    {
      MPI_Offset s;
      duneMPICall(MPI_File_get_size, *file_ptr_, &s);
      return s;
    }

    void preallocate(MPI_Offset s)
    {
      duneMPICall(MPI_File_preallocate, *file_ptr_, s);
    }

    void setInfo(MPI_Info i)
    {
      duneMPICall(MPI_File_set_info, *file_ptr_, i);
    }

    MPI_Info info()
    {
      MPI_Info i;
      duneMPICall(MPI_File_get_info, *file_ptr_, &i);
      return i;
    }

    int amode()
    {
      int amode;
      duneMPICall(MPI_File_get_amode, *file_ptr_, &amode);
      return amode;
    }

    void sync()
    {
      duneMPICall(MPI_File_sync, *file_ptr_);
    }

    void setAtomicity(bool flag)
    {
      duneMPICall(MPI_File_set_atomicity, *file_ptr_, flag);
    }

    bool atomicity() const
    {
      int flag;
      duneMPICall(MPI_File_get_atomicity, *file_ptr_, &flag);
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
      duneMPICall(MPI_File_seek, *file_ptr_, offset, (int)whence);
    }

    MPI_Offset position(){
      MPI_Offset offset;
      duneMPICall(MPI_File_get_position, *file_ptr_, &offset);
      return offset;
    }

    MPI_Offset byteOffset(MPI_Offset offset){
      MPI_Offset disp;
      duneMPICall(MPI_File_get_byte_offset, *file_ptr_, offset, &disp);
      return disp;
    }

    template<typename T>
    FutureType<std::decay_t<T>> iReadAt(MPI_Offset offset, T&& data)
    {
      FutureType<std::decay_t<T>> future(comm_, false, std::forward<T>(data));
      Span<std::decay_t<T>> span(future.buffer());
      duneMPICall(MPI_File_iread_at, *file_ptr_, offset,
                    span.ptr(), span.size(),
                    span.mpiType(), &future.mpiRequest());
      return future;
    }

    template<typename T>
    FutureType<> iWriteAt(MPI_Offset offset, const T& data)
    {
      FutureType<> future(comm_, false);
      Span<std::decay_t<T>> span(data);
      duneMPICall(MPI_File_iwrite_at, *file_ptr_, offset,
                    span.ptr(), span.size(),
                    span.mpiType(), &future.mpiRequest());
      return future;
    }

    template<typename T>
    FutureType<std::decay_t<T>> iRead(T&& data)
    {
      FutureType<std::decay_t<T>> future(comm_, false, std::forward<T>(data));
      Span<std::decay_t<T>> span(future.buffer());
      duneMPICall(MPI_File_iread, *file_ptr_,
                    span.ptr(), span.size(),
                    span.mpiType(),
                    &future.mpiRequest());
      return future;
    }

    template<typename T>
    FutureType<> iWrite(const T& data)
    {
      FutureType<> future(comm_, false);
      Span<std::decay_t<T>> span(data);
      duneMPICall(MPI_File_iwrite, *file_ptr_,
                    span.ptr(), span.size(),
                    span.mpiType(),
                    &future.mpiRequest());
      return future;
    }

#if MPI_VERSION > 3 || (MPI_VERSION==3 && MPI_SUBVERSION >=1)
    template<typename T>
    FutureType<std::decay_t<T>> iReadAtAll(MPI_Offset offset, T&& data)
    {
      FutureType<std::decay_t<T>> future(comm_, false, std::forward<T>(data));
      Span<std::decay_t<T>> span (future.buffer());
      duneMPICall(MPI_File_iread_at_all, *file_ptr_, offset,
                    span.ptr(), span.size(),
                    span.mpiType(), &future.mpiRequest());
      return future;
    }

    template<typename T>
    FutureType<> iWriteAtAll(MPI_Offset offset, const T& data)
    {
      FutureType<> future(comm_, false);
      Span<std::decay_t<T>> span(data);
      duneMPICall(MPI_File_iwrite_at_all, *file_ptr_, offset,
                    span.ptr(), span.size(),
                    span.mpiType(), &future.mpiRequest());
      return future;
    }

    template<typename T>
    FutureType<std::decay_t<T>> iReadAll(T&& data)
    {
      FutureType<std::decay_t<T>> future(comm_, false, std::forward<T>(data));
      Span<std::decay_t<T>> span(future.buffer());
      duneMPICall(MPI_File_iread_all, *file_ptr_,
                    span.ptr(), span.size(),
                    span.mpiType(),
                    &future.mpiRequest());
      return future;
    }

    template<typename T>
    FutureType<> iWriteAll(const T& data)
    {
      FutureType<> future(comm_, false);
      Span<std::decay_t<T>> span(data);
      duneMPICall(MPI_File_iwrite_all, *file_ptr_,
                    span.ptr(), span.size(),
                    span.mpiType(),
                    &future.mpiRequest());
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

  inline void deleteFile(const char* filename, MPI_Info info = MPI_INFO_NULL)
  {
    duneMPICall(MPI_File_delete, filename, info);
  }
}  // namespace Dune

#endif

#endif
