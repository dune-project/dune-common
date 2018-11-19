// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
/**
 * @file @brief See MPI_Pack. This Wrapper class takes care of the
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
#include <dune/common/parallel/mpidata.hh>


namespace Dune {

  class MPIPack {
    std::vector<char> _buffer;
    int _position;
    MPI_Comm _comm;

    friend struct MPIData<MPIPack>;
    friend struct MPIData<MPIPack&>;
    friend struct MPIData<const MPIPack&>;
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

    /** @brief Packs the data into the object.
     *
     * @throw MPIError
     */
    template<class T>
    void pack(const T& data){
      auto mpidata = getMPIData(data);
      int size = getPackSize(mpidata.size(), _comm, mpidata.type());
      if(!mpidata.static_size)
        size += getPackSize(1, _comm, MPI_INT);
      if (_position + size > 0 && size_t(_position + size) > _buffer.size()) // resize buffer if necessary
        _buffer.resize(_position + size);
      if(!mpidata.static_size){
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
    std::enable_if_t<MPIData<T>::static_size, void> unpack(T& data){
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
    std::enable_if_t<!MPIData<T>::static_size, void> unpack(T& data){
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

    /** @brief Resizes the internal buffer.
     */
    void resize(int size){
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
    void setPosition(int p){
      _position = p;
    }

    /** @brief Gets the position in the buffer where the next
     * pack/unpack operation should take place.
     */
    int getPosition() const{
      return _position;
    }

    /** @brief Checks whether the end of the buffer is reached.
     */
    bool atEnd() const{
      return std::size_t(_position)==_buffer.size();
    }

    /** @brief Returns the size of the data needed to store the data
     * in an MPIPack.
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

  template<class T>
  struct MPIData<T, std::enable_if_t<std::is_same<std::decay_t<T>, MPIPack>::value, void>> {
  protected:
    friend MPIData<T> getMPIData<T>(T&& t);
    MPIData(T&& t) :
      data_(std::forward<T>(t))
    {}
  public:
    static constexpr bool static_size = false;

    void* ptr() {
      return (void*) data_._buffer.data();
    }

    int size() {
      return data_.size();
    }

    MPI_Datatype type() const{
      return MPI_PACKED;
    }

    T get(){
      return std::forward<T>(data_);
    }

    void resize(int size){
      data_.resize(size);
    }
  protected:
    T data_;
  };

} // end namespace Dune

#endif
#endif
