// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_KERNEL_ALLOCATOR_HH
#define DUNE_COMMON_KERNEL_ALLOCATOR_HH

#include <memory>

#if defined HAVE_TBB
#include <tbb/tbb_allocator.h>
#include <tbb/cache_aligned_allocator.h>
#endif

#include <dune/common/typetraits.hh>
#include <dune/common/gcd.hh>

namespace Dune {
  namespace Kernel {


    template<typename T, typename size_t, size_t bs>
    struct blocked_std_allocator
      : public std::allocator<T>
    {

      typedef size_t size_type;

      static const size_type block_size = bs;

      static const size_type alignment = 1;

      template<typename U>
      struct rebind
      {
        typedef blocked_std_allocator<U,size_type,block_size> other;
      };

    };

#if defined HAVE_TBB

    template<typename T, typename size_t, size_t bs>
    struct blocked_cache_aligned_allocator
      : public tbb::cache_aligned_allocator<T>
    {

      typedef size_t size_type;

      static const size_type block_size = bs;

      static const size_type alignment = Dune::Gcd<
        TBB_CACHE_ALIGNED_ALLOCATOR_ALIGNMENT,
        block_size * sizeof(T)
        >::value;

      template<typename U>
      struct rebind
      {
        typedef blocked_cache_aligned_allocator<U,size_type,block_size> other;
      };

    };

    template<typename T, typename size_t, size_t bs>
    struct blocked_tbb_allocator
      : public tbb::tbb_allocator<T>
    {

      typedef size_t size_type;

      static const size_type block_size = bs;

      static const size_type alignment = 1;

      template<typename U>
      struct rebind
      {
        typedef blocked_tbb_allocator<U,size_type,block_size> other;
      };

    };

#endif

  } // namespace Kernel
} // namespace Dune

#endif // DUNE_COMMON_KERNEL_ALLOCATOR_HH
