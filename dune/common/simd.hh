#ifndef DUNE_COMMON_SIMD_HH
#define DUNE_COMMON_SIMD_HH

#include <dune/common/rangeutilities.hh>
#if HAVE_VC
#include <Vc/Vc>

namespace Dune
{
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
    bool b = false;
    for (std::size_t i = 0; i < v.size(); i++)
      b = b or bool(v[i]);
    return b;
  }

  template<typename T, std::size_t N, typename V, std::size_t M>
  bool any_true(const Vc::SimdMaskArray<T,N,V,M> & v)
  {
    bool b = false;
    for (std::size_t i = 0; i < v.size(); i++)
      b = b or bool(v[i]);
    return b;
  }

  template<typename T, typename A>
  bool all_true(const Vc::Mask<T,A> & v)
  {
    bool b = true;
    for (std::size_t i = 0; i < v.size(); i++)
      b = b and bool(v[i]);
    return b;
  }

  template<typename T, std::size_t N, typename V, std::size_t M>
  bool all_true(const Vc::SimdMaskArray<T,N,V,M> & v)
  {
    bool b = true;
    for (std::size_t i = 0; i < v.size(); i++)
      b = b and bool(v[i]);
    return b;
  }

}
#endif

#endif // DUNE_COMMON_SIMD_HH
