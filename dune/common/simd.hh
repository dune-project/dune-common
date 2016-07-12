#ifndef DUNE_COMMON_SIMD_HH
#define DUNE_COMMON_SIMD_HH

/**
   \file Abstractions for support of dedicated SIMD data types

   Libraries like Vc (https://github.com/VcDevel/Vc) add high-level
   data types for SIMD (or vectorization) support in C++.  Most of
   these operations mimic the behavior of a numerical data type. Some
   boolean operations can not be implemented in a compatible way to
   trivial data types.

   This header contains additional abstractions to help writing code
   that works with trivial numericaldata types (like double) and Vc
   vectorization data types.

   See also the conditional.hh and range_utils.hh headers.
 */

#include <dune/common/rangeutilities.hh>
#include <dune/common/conditional.hh>
#if HAVE_VC
#include <Vc/Vc>

namespace Dune
{

  template<typename T, typename A>
  Vc::Vector<T,A> cond(const Vc::Mask<T,A> & b,
    const Vc::Vector<T,A> & v1,
    const Vc::Vector<T,A> & v2)
  {
    return std::move(Vc::iif(b, v1, v2));
  }

  template<typename T, std::size_t N, typename V, std::size_t M>
  Vc::SimdArray<T,N,V,M> cond(const typename Vc::SimdArray<T,N,V,M>::mask_type & b,
    const Vc::SimdArray<T,N,V,M> & v1,
    const Vc::SimdArray<T,N,V,M> & v2)
  {
    return std::move(Vc::iif(b, v1, v2));
  }

  template<typename T, typename A>
  T max_value(const Vc::Vector<T,A> & v)
  {
    return v.max();
  }

  template<typename T, std::size_t N, typename V, std::size_t M>
  double max_value(const Vc::SimdArray<T,N,V,M> & v)
  {
    return v.max();
  }

  template<typename T, typename A>
  T min_value(const Vc::Vector<T,A> & v)
  {
    return v.min();
  }

  template<typename T, std::size_t N, typename V, std::size_t M>
  double min_value(const Vc::SimdArray<T,N,V,M> & v)
  {
    return v.min();
  }

  template<typename T, typename A>
  bool any_true(const Vc::Mask<T,A> & v)
  {
    return Vc::any_of(v);
  }

  template<typename T, std::size_t N, typename V, std::size_t M>
  bool any_true(const Vc::SimdMaskArray<T,N,V,M> & v)
  {
    return Vc::any_of(v);
  }

  template<typename T, typename A>
  bool all_true(const Vc::Mask<T,A> & v)
  {
    return Vc::all_of(v);
  }

  template<typename T, std::size_t N, typename V, std::size_t M>
  bool all_true(const Vc::SimdMaskArray<T,N,V,M> & v)
  {
    return Vc::all_of(v);
  }

} // end namespace Dune
#endif // HAVE_VC

#endif // DUNE_COMMON_SIMD_HH
