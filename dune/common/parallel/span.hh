// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

/**
 * @file @brief Interface for providing memory information for the MPI
 * function calls (pointer, length, MPI_Datatype). Every object, that
 * can be communicated via MPI is convertible to Span<T>.
 * Additionally it provides a interface for resizing the Datatype,
 * s.t. the received data fits into the memory.
 *
 * @author Nils-Arne Dreier
 * @ingroup ParallelCommunication
 */

#ifndef DUNE_COMMON_PARALLEL_SPAN_HH
#define DUNE_COMMON_PARALLEL_SPAN_HH

#include <vector>
#include <memory>
#include <string>
#include <type_traits>
#include "mpitraits.hh"

namespace Dune {
  // Generic implementation (for instance scalars)
  template<typename T, typename = void>
  class Span
  {
  public:
    typedef std::remove_cv_t<T> type;
    Span(const T& t) : ptr_(const_cast<std::remove_cv_t<T>*>(&t)) {}
    Span(const Span&) = default;

    // accessors:
#if HAVE_MPI
    static constexpr MPI_Datatype mpiType(){
      return MPITraits<std::remove_cv_t<T>>::getType();
    }
#endif

    type* ptr() const {
      return ptr_;
    }

    static constexpr size_t size() {
      return 1;
    }

    /** @brief Indicated whether the underlying object has dynamic
     * size and is resizeable by this wrapper class.
     */
    static constexpr bool dynamicSize = false;

    void resize(size_t s){
      if(s != 1)
        DUNE_THROW(Exception, "This function does not make any sense here");
    }
  protected:
    // member variables
    type* ptr_;
  };

  // Specialization for C-Arrays
  template<class T>
  class Span<T*> {
  protected:
    std::remove_cv_t<T*> ptr_;
    size_t len_;

  public:
    typedef std::remove_cv_t<T> type;
    Span(T* arr, std::size_t l) : ptr_(arr), len_(l)
    {}

#if HAVE_MPI
    // accessors:
    static constexpr MPI_Datatype mpiType(){
      return MPITraits<std::remove_cv_t<T>>::getType();
    }
#endif

    type* ptr() const {
      return ptr_;
    }

    size_t size() const {
      return len_;
    }

    static constexpr bool dynamicSize = false;
    // this function should never be called:
    void resize(size_t s){
      if(s != len_)
        DUNE_THROW(Exception, "This function does not make any sense here");
    }
  };


  // Specialization for Span types
  // (e.g. C-array that are already wrapped in a Span)
  template<class T>
  struct Span<Span<T*>> : public Span<T*>
  {
    typedef std::remove_cv_t<T> type;
    Span(const Span<Span<T*>>& s) = default;
    Span(Span<T*>& s) : Span<T*>(s.ptr(), s.size())
    {}
  };

  template<class T>
  struct Span<const Span<T*>> : public Span<T*>
  {
    typedef std::remove_cv_t<T> type;
    Span(const Span<const Span<T*>>& s) = default;
    Span(const Span<T*>& s) : Span<T*>(s.ptr(), s.size())
    {}
  };

  // Specializations for pointer types
  template<class T>
  struct Span<std::shared_ptr<T>> : public Span<T>
  {
    typedef typename Span<T>::type type;
    Span(const Span<std::shared_ptr<T>>& s) = default;
    Span(std::shared_ptr<T>& p) : Span<T>(*p) {}
  };

  template<class T, class D>
  struct Span<std::unique_ptr<T, D>> : public Span<T>
  {
    typedef typename Span<T>::type type;
    Span(const Span&) = default;
    Span(std::unique_ptr<T, D>& p) : Span<T>(*p) {}
  };

  // specialization for dynamic-sized vectors
  template<typename T> struct isVector : std::false_type {};
  template<typename T, typename A> struct isVector<std::vector<T, A>> : std::true_type {};
  template<typename T, typename A> struct isVector<const std::vector<T,A>> : std::true_type {};
  // std::string satisfies the same interface
  template<> struct isVector<std::string> : std::true_type {};
  template<> struct isVector<const std::string> : std::true_type {};

  template<class T>
  struct Span<T, std::enable_if_t<isVector<T>::value>>
  {
    typedef typename T::value_type value_type;
    typedef typename Span<value_type>::type type;
    Span(T& v)
      : vec_(v)
    {}

#if HAVE_MPI
    static constexpr MPI_Datatype mpiType(){
      return Span<std::remove_cv_t<value_type>>::mpiType();
    }
#endif

    type* ptr() const{
      return reinterpret_cast<type*>(const_cast<value_type*>(vec_.data()));
    }

    size_t size() const {
      return vec_.size();
    }

    static constexpr bool dynamicSize = !std::is_const<T>::value;
    void resize(size_t s){
      static_assert(dynamicSize, "This object can't be resized.");
      vec_.resize(s);
    }

    operator Span<value_type*> () const {
      return {ptr(), size()};
    }

  protected:
    T& vec_; // keep track of the object (the data
                          // pointer might change if the vector is
                          // resized)
  };
}

#endif
