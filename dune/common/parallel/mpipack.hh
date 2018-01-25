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

#if HAVE_MPI

#include <type_traits>
#include <vector>

#include <mpi.h>

#include "mpitraits.hh"
#include "span.hh"
#include "mpiexceptions.hh"

namespace Dune {

  template<typename C>
  class MPIPack
  {
    friend class Span<MPIPack>;
    friend class Span<const MPIPack>;
    std::vector<char> buffer_;
    int position_;
    C comm_;

  public:
    explicit MPIPack(const C& c, size_t size = 0)
      : buffer_(size),
        position_(0),
        comm_(c)
    {}

    MPIPack(const MPIPack&) = default;
    MPIPack(MPIPack&&) = default;
    MPIPack& operator = (MPIPack&& other) = default;

    /** @brief Packs the data into the object.
     *
     * @throw MPIError
     */
    template<class T>
    void pack(const T& data){
      Span<const T> span(data);
      int data_size = 0;
      dune_mpi_call(MPI_Pack_size, span.size(),
                    span.mpi_type(), comm_, &data_size);
      if (position_ + data_size > 0 && size_t(position_ + data_size) > buffer_.size())
        buffer_.resize(position_ + data_size);
      dune_mpi_call(MPI_Pack, span.ptr(), span.size(),
                    span.mpi_type(), buffer_.data(), buffer_.size(),
                    &position_, comm_);
    }

    /** @brief Unpacks data from the object
     *
     * @throw MPIError
     */
    template<class T>
    void unpack(T& data){
      Span<T> span(data);
      dune_mpi_call(MPI_Unpack, buffer_.data(), buffer_.size(), &position_,
                    span.ptr(), span.size(),
                    span.mpi_type(), comm_);
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

    /** @brief Resizes the internal buffer_.
     */
    void resize(int size){
      buffer_.resize(size);
    }

    /** @brief Enlarges the internal buffer_.
     */
    void enlarge(int s) {
      buffer_.resize(buffer_.size() + s);
    }

    /** @brief Returns the size of the internal buffer_.
     */
    size_t size() const {
      return buffer_.size();
    }

    /** @brief Sets the position_ in the buffer_ where the next
     * pack/unpack operation should take place.
    */
    void set_position(int p){
      position_ = p;
    }

    /** @brief Gets the position_ in the buffer_ where the next
     * pack/unpack operation should take place.
    */
    int get_position() const{
      return position_;
    }

    /** @brief Checks whether the end of the buffer_ is reached.
     */
    bool at_end() const{
      return position_==buffer_.size();
    }

    /** @brief Returns the size of the data needed to store the data
     * in an MPIPack.
     */
    static int get_pack_size(int len, const C& comm, const MPI_Datatype& dt){
      int size;
      dune_mpi_call(MPI_Pack_size, len, dt, comm, &size);
      return size;
    }

    friend bool operator==(const MPIPack& a, const MPIPack& b) {
      return  a.buffer_ == b.buffer_ && a.comm_ == b.comm_;
    }
    friend bool operator!=(const MPIPack& a, const MPIPack& b) {
      return !(a==b);
    }
  };

  template<typename> struct is_MPIPack : std::false_type {};
  template<typename C> struct is_MPIPack<MPIPack<C>> : std::true_type {};
  template<typename C> struct is_MPIPack<const MPIPack<C>> : std::true_type {};

  // Specialization for MPIPack
  template<typename P>
  struct Span<P, std::enable_if_t<is_MPIPack<P>::value>> {
    typedef P type;
    Span(P& p)
      : pack_(p)
    {}

    static constexpr MPI_Datatype mpi_type(){
      return MPI_PACKED;
    }

    P* ptr() const {
      return reinterpret_cast<P*>(pack_.buffer_.data());
    }

    size_t size() const {
      return pack_.buffer_.size();
    }

    static constexpr bool dynamic_size = !std::is_const<P>::value;
    void resize(size_t s){
      static_assert(dynamic_size, "MPIPack can't be resized.");
      pack_.buffer_.resize(s);
    }

  protected:
    P& pack_;
  };
}  // namespace Dune

#endif

#endif
