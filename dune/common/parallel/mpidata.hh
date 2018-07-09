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
 * @file @brief Interface class to translate objects to a
 * MPI_Datatype, void* and size used for MPI calls. It can be used for
 * values types as well as for reference types. Furthermore it keeps
 * the object until it is deconstructed. Static datatypes are
 * supported by `Dune::MPITraits`.
 *
 * To 'register' a new dynamic type for MPI communication just specialize MPIData.
 *
 */

namespace Dune{

  // Default implementation for static datatypes
  template<class T, class Enable = void>
  struct MPIData
  {

    MPIData(T&& t)
      : data_(std::forward<T>(t))
    {}

    void* ptr() const {
      return (void*)&data_;
    }

    static constexpr bool static_size = true;

    int size() const{
      return 1;
    }

    T get(){
      return std::forward<T>(data_);
    }

    MPI_Datatype type() const {
      return MPITraits<T>::getType();
    }
  protected:
    T data_;
  };


  // free function for template argument deduction
  template<class T>
  MPIData<T> getMPIData(T&& t){
    return MPIData<T>(std::forward<T>(t));
  }

  // dummy implementation for void
  template<>
  struct MPIData<void>{
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

    MPIData(T&& t)
      : data_(std::forward<T>(t))
    {}

    static constexpr bool static_size = false;
    void* ptr() {
      return (void*) data_.data();
    }
    int size() {
      return data_.size();
    }
    MPI_Datatype type() const{
      return MPITraits<typename std::decay_t<T>::value_type>::getType();
    }
    T get(){
      return std::forward<T>(data_);
    }

  protected:
    T data_;
  };


  // Dune::DynamicVector
  template<class K, class Allocator> class DynamicVector;
  template<class V> struct is_DynamicVector : std::false_type{};
  template<class K, class Allocator> struct is_DynamicVector<DynamicVector<K, Allocator>> : std::true_type{};
  template<class T>
  struct MPIData<T, std::enable_if_t<is_DynamicVector<std::decay_t<T>>::value>>
  {
    MPIData(T&& t)
      : data_(std::forward<T>(t))
    {}

    static constexpr bool static_size = false;

    void* ptr() {
      return (void*) data_.container().data();
    }

    int size() {
      return data_.size();
    }

    MPI_Datatype type() const{
      return MPITraits<typename std::decay_t<T>::value_type>::getType();
    }

    T get(){
      return std::forward<T>(data_);
    }
  protected:
    T data_;
  };

}

#endif
#endif
