// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifndef DUNE_COMMON_PARALLEL_MPIDATA_HH
#define DUNE_COMMON_PARALLEL_MPIDATA_HH

#include <vector>
#include <string>

#if HAVE_MPI

#include <dune/common/typetraits.hh>
#include <dune/common/std/type_traits.hh>
#include <dune/common/parallel/mpitraits.hh>

/** @addtogroup ParallelCommunication
 *
 * @{
 */
/**
 * @file
 *
 * @brief Interface class to translate objects to a MPI_Datatype, void*
 * and size used for MPI calls.
 *
 * Furthermore it can be used to resize the object
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
      return MPITraits<std::decay_t<T>>::getType();
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
  struct MPIData<T, std::void_t<std::tuple<decltype(std::declval<T>().data()),
                                           decltype(std::declval<T>().size()),
                                           typename std::decay_t<T>::value_type>>>{
  private:
    template<class U>
    using hasResizeOp = decltype(std::declval<U>().resize(0));

  protected:
    friend auto getMPIData<T>(T&);
    MPIData(T& t)
      : data_(t)
    {}
  public:
    static constexpr bool static_size = std::is_const<T>::value || !Std::is_detected_v<hasResizeOp, T>;
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
      -> std::enable_if_t<!std::is_const<S>::value || !Std::is_detected_v<hasResizeOp, S>>
    {
      data_.resize(size);
    }

  protected:
    T& data_;
  };

}

/**
 * @}
 */

#endif
#endif
