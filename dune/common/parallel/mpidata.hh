// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_PARALLEL_MPIDATA_HH
#define DUNE_COMMON_PARALLEL_MPIDATA_HH

#include <vector>
#include <string>

#if HAVE_MPI

#include <dune/common/parallel/mpitraits.hh>

/** @addtogroup ParallelCommunication
 *
 * @{
 */
/**
 * @file @brief Interface class to translate objects to a MPI_Datatype, void*
 * and size used for MPI calls. Furthermore it can be used to resize the object
 * if possible. This makes it possible to receive a message with variable
 * size. See `Communication::rrecv`.
 *
 * To 'register' a new dynamic type for MPI communication specialize `MPIData` or
 * overload `getMPIData`.
 *
 */

namespace Dune{

  template<class, class = void>
  struct MPIData;

  template<class T>
  auto getMPIData(T& t){
    return MPIData<T>(t);
  }

  // Default implementation for static datatypes
  template<class T, class Enable>
  struct MPIData
  {
    friend auto getMPIData<T>(T&);
  protected:
    T& data_;

    MPIData(T& t)
      : data_(t)
    {}

  public:
    void* ptr() const {
      return (void*)&data_;
    }

    // indicates whether the datatype can be resized
    static constexpr bool static_size = true;

    int size() const{
      return 1;
    }

    MPI_Datatype type() const {
      return MPITraits<T>::getType();
    }
  };

  // dummy implementation for void
  template<>
  struct MPIData<void>{
  protected:
    MPIData() {}

  public:
    void* ptr(){
      return nullptr;
    }
    int size(){
      return 0;
    }
    void get(){}
    MPI_Datatype type() const{
      return MPI_INT;
    }
  };

  // specializations:
  // std::vector of static sized elements or std::string
  template<class T>
  struct MPIData<T, std::enable_if_t<
                      (std::is_same<std::decay_t<T>, std::vector<typename std::decay_t<T>::value_type>>::value
                       && MPIData<typename std::decay_t<T>::value_type>::static_size)
                      || std::is_same<std::decay_t<T>, std::string>::value>>{

  protected:
    friend auto getMPIData<T>(T&);
    MPIData(T& t)
      : data_(t)
    {}
  public:
    static constexpr bool static_size = std::is_const<T>::value;
    void* ptr() {
      return (void*) data_.data();
    }
    int size() {
      return data_.size();
    }
    MPI_Datatype type() const{
      return MPITraits<typename std::decay_t<T>::value_type>::getType();
    }

    template<class S = T>
    auto /*void*/ resize(int size)
      -> std::enable_if_t<!std::is_const<S>::value>
    {
      data_.resize(size);
    }

  protected:
    T& data_;
  };


  // Dune::DynamicVector
  template<class K, class Allocator> class DynamicVector;
  template<class V> struct is_DynamicVector : std::false_type{};
  template<class K, class Allocator> struct is_DynamicVector<DynamicVector<K, Allocator>> : std::true_type{};
  template<class T>
  struct MPIData<T, std::enable_if_t<is_DynamicVector<std::decay_t<T>>::value>>
  {
  protected:
    friend auto getMPIData<T>(T&);

    T& data_;

    MPIData(T& t)
      : data_(t)
    {}
  public:
    static constexpr bool static_size = std::is_const<T>::value;

    void* ptr() {
      return (void*) data_.container().data();
    }

    int size() {
      return data_.size();
    }

    MPI_Datatype type() const{
      return MPITraits<typename std::decay_t<T>::value_type>::getType();
    }

    template<class S = T>
    auto /*void*/ resize(int size)
      -> std::enable_if_t<!std::is_const<S>::value>
    {
      data_.resize(size);
    }
  };

}

#endif
#endif
