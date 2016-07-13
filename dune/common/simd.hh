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
#endif

namespace Dune
{

  template<typename T>
  struct SimdScalarTypeTraits
  {
    using type = T;
  };

  template<typename T>
  using SimdScalar = typename SimdScalarTypeTraits<T>::type;

#if HAVE_VC
  /*
    Add Vc specializations for the SimdScalarTypeTraits trais class
   */
  template<typename T, typename A>
  struct SimdScalarTypeTraits< Vc::Vector<T,A> >
  {
    using type = T;
  };

  template<typename T, std::size_t N, typename V, std::size_t M>
  struct SimdScalarTypeTraits< Vc::SimdArray<T,N,V,M> >
  {
    using type = T;
  };
#endif // HAVE_VC

#if HAVE_VC
  /*
    Add Vc specializations for cond(), see conditional.hh
   */
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
#endif // HAVE_VC

#if HAVE_VC
  /*
    Add Vc specializations for several boolean operations, see rangeutitlities.hh:

    max_value, min_value, any_true, all_true
   */
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
#endif // HAVE_VC

} // end namespace Dune

#endif // DUNE_COMMON_SIMD_HH
