// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_STREAMOPERATORS_HH
#define DUNE_STREAMOPERATORS_HH

/** \file
    \brief Implementation of stream operators for std::array and std::tuple
 */

#include <array>
#include <tuple>

#include <dune/common/hybridutilities.hh>
#include <dune/common/std/utility.hh>

namespace Dune
{
  /** @addtogroup Common

     @{
   */

  //! Print a std::tuple
  template<typename Stream, typename... Ts>
  inline Stream& operator<<(Stream& stream, const std::tuple<Ts...>& t)
  {
    stream<<"[";
    if(sizeof...(Ts)>0)
    {
      Hybrid::forEach(Std::make_index_sequence<sizeof...(Ts)-1>{},
        [&](auto i){stream<<std::get<i>(t)<<",";});
      stream<<std::get<sizeof...(Ts)-1>(t);
    }
    stream<<"]";
    return stream;
  }

  //! Read a std::tuple
  template<typename Stream, typename... Ts>
  inline Stream& operator>>(Stream& stream, std::tuple<Ts...>& t)
  {
    Hybrid::forEach(Std::make_index_sequence<sizeof...(Ts)>{},
      [&](auto i){stream>>std::get<i>(t);});
    return stream;
  }

  //! Print a std::array
  template<typename Stream, typename T, std::size_t N>
  inline Stream& operator<<(Stream& stream, const std::array<T,N>& a)
  {
    stream<<"[";
    if(N>0)
    {
      for(std::size_t i=0; i<N-1; ++i)
        stream<<a[i]<<",";
      stream<<a[N-1];
    }
    stream<<"]";
    return stream;
  }

  /** @} */

} // end namespace Dune

#endif
