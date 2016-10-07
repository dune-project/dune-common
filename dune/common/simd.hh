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

#include <cassert>
#include <cstddef>
#include <utility>

#if HAVE_VC
#include <Vc/Vc>
#endif

#include <dune/common/conditional.hh>
#include <dune/common/rangeutilities.hh>

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

  template<typename V, typename = void>
  struct SimdIndexTypeTraits {
    using type = std::size_t;
  };

  //! An simd vector of indices corresponding to a simd vector V
  /**
   * lanes(T()) == lanes(SimdIndex<T>()) holds.
   *
   * \note The size of the elements of a SimdIndex isn't very well-defined.
   *       Be careful.
   */
  template<typename V>
  using SimdIndex = typename SimdIndexTypeTraits<V>::type;

#if HAVE_VC
  template<typename T, typename A>
  struct SimdIndexTypeTraits<Vc::Vector<T, A> > {
    using type = typename Vc::Vector<T, A>::index_type;
  };

  template<typename T, std::size_t n, typename V>
  struct SimdIndexTypeTraits<Vc::SimdArray<T, n, V> > {
    using type = typename Vc::SimdArray<T, n, V>::index_type;
  };
#endif // HAVE_VC

  template<typename V, typename = void>
  struct SimdMaskTypeTraits {
    using type = bool;
  };

  //! A simd vector of truth values corresponding to a simd vector V
  /**
   * lanes(T()) == lanes(SimdMask<T>()) holds.
   */
  template<typename V>
  using SimdMask = typename SimdMaskTypeTraits<V>::type;

#if HAVE_VC
  template<typename T, typename A>
  struct SimdMaskTypeTraits<Vc::Vector<T, A> > {
    using type = typename Vc::Vector<T, A>::mask_type;
  };

  template<typename T, std::size_t n, typename V>
  struct SimdMaskTypeTraits<Vc::SimdArray<T, n, V> > {
    using type = typename Vc::SimdArray<T, n, V>::mask_type;
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

  //! get the number of lanes of a simd vector (scalar version)
  template<class T>
  std::size_t lanes(const T &) { return 1; }

  //! access a lane of a simd vector (scalar version)
  template<class T>
  T lane(std::size_t l, const T &v)
  {
    assert(l == 0);
    return v;
  }

  //! access a lane of a simd vector (scalar version)
  template<class T>
  T &lane(std::size_t l, T &v)
  {
    assert(l == 0);
    return v;
  }

#if HAVE_VC
  template<class T, class A>
  std::size_t lanes(const Vc::Vector<T, A> &)
  {
    return Vc::Vector<T, A>::size();
  }

  template<class T, class A>
  T lane(std::size_t l, const Vc::Vector<T, A> &v)
  {
    assert(l < lanes(v));
    return v[l];
  }

  template<class T, class A>
  T &lane(std::size_t l, Vc::Vector<T, A> &v)
  {
    assert(l < lanes(v));
    return v[l];
  }

  template<class T, std::size_t n, class V>
  std::size_t lanes(const Vc::SimdArray<T, n, V> &)
  {
    return n;
  }

  template<class T, std::size_t n, class V>
  const T lane(std::size_t l, const Vc::SimdArray<T, n, V> &v)
  {
    assert(l < n);
    return v[l];
  }

  template<class T, std::size_t n, class V>
  T &lane(std::size_t l, Vc::SimdArray<T, n, V> &v)
  {
    assert(l < n);
    return v[l];
  }

  template<class T, std::size_t n, class V>
  std::size_t lanes(const Vc::SimdMaskArray<T, n, V> &)
  {
    return n;
  }

  template<class T, std::size_t n, class V>
  const bool lane(std::size_t l, const Vc::SimdMaskArray<T, n, V> &v)
  {
    assert(l < n);
    return v[l];
  }

  template<class T, std::size_t n, class V>
  auto lane(std::size_t l, Vc::SimdMaskArray<T, n, V> &v)
    -> decltype(v[l])
  {
    assert(l < n);
    return v[l];
  }
#endif // HAVE_VC

  //! masked Simd assignment (scalar version)
  /**
   * Assign \c src to \c dest for those lanes where \c mask is true.
   */
  template<class T>
  void assign(T &dst, const T &src, bool mask)
  {
    if(mask) dst = src;
  }

#if HAVE_VC
  /*
    Add Vc specializations for masked assignment
  */
  template<class T, class A>
  void assign(Vc::Vector<T, A> &dst, const Vc::Vector<T, A> &src,
              typename Vc::Vector<T, A>::mask_type mask)
  {
    dst(mask) = src;
  }

  template<class T, std::size_t n, class V>
  void assign(Vc::SimdArray<T, n, V> &dst, const Vc::SimdArray<T, n, V> &src,
              typename Vc::SimdArray<T, n, V>::mask_type mask)
  {
    dst(mask) = src;
  }
#endif // HAVE_VC

  template<class T>
  void swap(T &v1, T &v2, bool mask)
  {
    using std::swap;
    if(mask) swap(v1, v2);
  }

#if HAVE_VC
  /*
    Add Vc specializations for masked swap
  */
  template<class T, class A>
  void swap(Vc::Vector<T, A> &v1, Vc::Vector<T, A> &v2,
            typename Vc::Vector<T, A>::mask_type mask)
  {
    auto tmp = v1;
    v1(mask) = v2;
    v2(mask) = tmp;
  }

  template<class T, std::size_t n, class V>
  void swap(Vc::SimdArray<T, n, V> &v1, Vc::SimdArray<T, n, V> &v2,
            typename Vc::SimdArray<T, n, V>::mask_type mask)
  {
    auto tmp = v1;
    v1(mask) = v2;
    v2(mask) = tmp;
  }
#endif // HAVE_VC

} // end namespace Dune

#endif // DUNE_COMMON_SIMD_HH
