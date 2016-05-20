// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_MEMORY_ALIGNMENT_HH
#define DUNE_COMMON_MEMORY_ALIGNMENT_HH

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <dune/common/typetraits.hh>

#if defined __INTEL_COMPILER
// this is messy as hell, but we typically feed a template argument to alignment, and ICC doesn't like that
#define DUNE_ASSUME_ALIGNED(x,T,alignment) __assume_aligned(x,64)
#elif defined __clang__
#define DUNE_ASSUME_ALIGNED(x,T,alignment)
#elif defined __GNUC__
#define DUNE_ASSUME_ALIGNED(x,T,alignment) x = static_cast<T*>(__builtin_assume_aligned(x,alignment))
#else
#define DUNE_ASSUME_ALIGNED(x,T,alignment)
#endif

namespace Dune {
  namespace Memory {

#if defined HAVE_ALIGNOF

    template<typename T>
    struct align_of
      : public std::integral_constant<std::size_t,alignof(T)>
    {};

#else

    // we have no idea, so let's be conservative
    template<typename T>
    struct align_of
      : public std::integral_constant<std::size_t,1>
    {};

    template<>
    struct align_of<std::size_t>
      : public std::integral_constant<std::size_t,ALIGNOF_SIZE_T>
    {};

    template<>
    struct align_of<std::uint32_t>
      : public std::integral_constant<std::size_t,ALIGNOF_UINT32_T>
    {};

    template<>
    struct align_of<double>
      : public std::integral_constant<std::size_t,ALIGNOF_DOUBLE>
    {};

    template<>
    struct align_of<float>
      : public std::integral_constant<std::size_t,ALIGNOF_FLOAT>
    {};

#endif // HAVE_ALIGNOF


  } // namespace Memory
} //namespace Dune

#endif // DUNE_COMMON_MEMORY_ALIGNMENT_HH
