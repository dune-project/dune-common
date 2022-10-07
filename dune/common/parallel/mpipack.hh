// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
/**
 * @file
 *
 * @brief See MPI_Pack.
 *
 * This Wrapper class takes care of the
 * memory management and provides methods to pack and unpack
 * objects. All objects that can be used for MPI communication can
 * also be packed and unpacked to/from MPIPack.
 *
 * @author Nils-Arne Dreier
 * @ingroup ParallelCommunication
 */


#ifndef DUNE_COMMON_PARALLEL_MPIPACK_HH
#define DUNE_COMMON_PARALLEL_MPIPACK_HH

#include <vector>
#if HAVE_MPI
#include <mpi.h>
#include <dune/common/parallel/mpicommunication.hh>
#include <dune/common/parallel/mpidata.hh>


namespace Dune {

  class MPIPack {
    std::vector<char> _buffer;
    int _position;
    MPI_Comm _comm;

    friend struct MPIData<MPIPack>;
    friend struct MPIData<const MPIPack>;
  public:
    MPIPack(Communication<MPI_Comm> comm, std::size_t size = 0)
      : _buffer(size)
      , _position(0)
      , _comm(comm)
    {}

    // Its not valid to copy a MPIPack but you can move it
    MPIPack(const MPIPack&) = delete;
    MPIPack& operator = (const MPIPack& other) = delete;
    MPIPack(MPIPack&&) = default;
    MPIPack& operator = (MPIPack&& other) = default;

    /** @brief Packs the data into the object. Enlarges the internal buffer if
     * necessary.
     *
     * @throw MPIError
     */
    template<class T>
    void pack(const T& data){
      auto mpidata = getMPIData(data);
      int size = getPackSize(mpidata.size(), _comm, mpidata.type());
      constexpr bool has_static_size = decltype(getMPIData(std::declval<T&>()))::static_size;
      if(!has_static_size)
        size += getPackSize(1, _comm, MPI_INT);
      if (_position + size > 0 && size_t(_position + size) > _buffer.size()) // resize buffer if necessary
        _buffer.resize(_position + size);
      if(!has_static_size){
        int size = mpidata.size();
        MPI_Pack(&size, 1, MPI_INT, _buffer.data(), _buffer.size(),
                 &_position, _comm);
      }
      MPI_Pack(mpidata.ptr(), mpidata.size(),
               mpidata.type(), _buffer.data(), _buffer.size(),
               &_position, _comm);
    }

    /** @brief Unpacks data from the object
     *
     * @throw MPIError
     */
    template<class T>
    auto /*void*/ unpack(T& data)
      -> std::enable_if_t<decltype(getMPIData(data))::static_size, void>
    {
      auto mpidata = getMPIData(data);
      MPI_Unpack(_buffer.data(), _buffer.size(), &_position,
                 mpidata.ptr(), mpidata.size(),
                 mpidata.type(), _comm);
    }

    /** @brief Unpacks data from the object
     *
     * @throw MPIError
     */
    template<class T>
    auto /*void*/ unpack(T& data)
      -> std::enable_if_t<!decltype(getMPIData(data))::static_size, void>
    {
      auto mpidata = getMPIData(data);
      int size = 0;
      MPI_Unpack(_buffer.data(), _buffer.size(), &_position,
                 &size, 1,
                 MPI_INT, _comm);
      mpidata.resize(size);
      MPI_Unpack(_buffer.data(), _buffer.size(), &_position,
                 mpidata.ptr(), mpidata.size(),
                 mpidata.type(), _comm);
    }


    //! @copydoc pack
    template<typename T>
    friend MPIPack& operator << (MPIPack& p, const T& t){
      p.pack(t);
      return p;
    }

    //! @copydoc unpack
    template<typename T>
    friend MPIPack& operator >> (MPIPack& p, T& t){
      p.unpack(t);
      return p;
    }

    //! @copydoc unpack
    template<typename T>
    MPIPack& read(T& t){
      unpack(t);
      return *this;
    }

    //! @copydoc pack
    template<typename T>
    MPIPack& write(const T& t){
      pack(t);
      return *this;
    }

    /** @brief Resizes the internal buffer.
        \param size new size of internal buffer
     */
    void resize(size_t size){
      _buffer.resize(size);
    }

    /** @brief Enlarges the internal buffer.
     */
    void enlarge(int s) {
      _buffer.resize(_buffer.size() + s);
    }

    /** @brief Returns the size of the internal buffer.
     */
    size_t size() const {
      return _buffer.size();
    }

    /** @brief Sets the position in the buffer where the next
     * pack/unpack operation should take place.
     */
    void seek(int p){
      _position = p;
    }

    /** @brief Gets the position in the buffer where the next
     * pack/unpack operation should take place.
     */
    int tell() const{
      return _position;
    }

    /** @brief Checks whether the end of the buffer is reached.
     */
    bool eof() const{
      return std::size_t(_position)==_buffer.size();
    }

    /** @brief Returns the size of the data needed to store the data
     * in an MPIPack. See `MPI_Pack_size`.
     */
    static int getPackSize(int len, const MPI_Comm& comm, const MPI_Datatype& dt){
      int size;
      MPI_Pack_size(len, dt, comm, &size);
      return size;
    }

    friend bool operator==(const MPIPack& a, const MPIPack& b) {
      return  a._buffer == b._buffer && a._comm == b._comm;
    }
    friend bool operator!=(const MPIPack& a, const MPIPack& b) {
      return !(a==b);
    }

  };

  template<class P>
  struct MPIData<P, std::enable_if_t<std::is_same<std::remove_const_t<P>, MPIPack>::value>> {
  protected:
    friend auto getMPIData<P>(P& t);
    MPIData(P& t) :
      data_(t)
    {}
  public:
    static constexpr bool static_size = std::is_const<P>::value;

    void* ptr() {
      return (void*) data_._buffer.data();
    }

    int size() {
      return data_.size();
    }

    MPI_Datatype type() const{
      return MPI_PACKED;
    }

    void resize(int size){
      data_.resize(size);
    }
  protected:
    P& data_;
  };

} // end namespace Dune

#endif
#endif
