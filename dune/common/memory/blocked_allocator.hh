// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_COMMON_MEMORY_BLOCKED_ALLOCATOR_HH
#define DUNE_COMMON_MEMORY_BLOCKED_ALLOCATOR_HH

#include <memory>

#if defined HAVE_TBB

#include <tbb/tbb_allocator.h>
#include <tbb/cache_aligned_allocator.h>

#if defined HAVE_TBB_ALLOCATOR
#include <tbb/scalable_allocator.h>
#endif

#endif

#include <dune/common/typetraits.hh>
#include <dune/common/gcd.hh>
#include <dune/common/memory/domain.hh>
#include <dune/common/memory/alignment.hh>

namespace Dune {
  namespace Memory {


    template<typename T, typename size_t, size_t bs>
    struct blocked_std_allocator
      : public std::allocator<T>
    {

      typedef size_t size_type;

      static const size_type block_size = bs;

      static const size_type alignment = alignment<T>::value;

      static const size_type alignment_for_block_size = Dune::Gcd<
        alignment,
        block_size * sizeof(T)
        >::value;

      static const size_type minimum_chunk_size = block_size * alignment / alignment_for_block_size;

      template<typename U>
      struct rebind
      {
        typedef blocked_std_allocator<U,size_type,block_size> other;
      };

    };

    template<typename T, typename size_t, size_t bs>
    struct allocator_domain<
      blocked_std_allocator<T,size_t,bs>
      >
    {
      typedef Domain::Host type;
    };

#if defined HAVE_TBB

    template<typename T, typename size_t, size_t bs>
    struct blocked_cache_aligned_allocator
      : public tbb::cache_aligned_allocator<T>
    {

      typedef size_t size_type;

      static const size_type block_size = bs;

      static const size_type alignment = TBB_CACHE_ALIGNED_ALLOCATOR_ALIGNMENT;

      static const size_type alignment_for_block_size = Dune::Gcd<
        alignment,
        block_size * sizeof(T)
        >::value;

      static const size_type minimum_chunk_size = block_size * alignment / alignment_for_block_size;

      template<typename U>
      struct rebind
      {
        typedef blocked_cache_aligned_allocator<U,size_type,block_size> other;
      };

    };

    template<typename T, typename size_t, size_t bs>
    struct allocator_domain<
      blocked_cache_aligned_allocator<T,size_t,bs>
      >
    {
      typedef Domain::Host type;
    };


    template<typename T, typename size_t, size_t bs>
    struct blocked_tbb_allocator
      : public tbb::tbb_allocator<T>
    {

      typedef size_t size_type;

      static const size_type block_size = bs;

      static const size_type alignment = alignment<T>::value;

      static const size_type alignment_for_block_size = Dune::Gcd<
        alignment,
        block_size * sizeof(T)
        >::value;

      static const size_type minimum_chunk_size = block_size * alignment / alignment_for_block_size;

      template<typename U>
      struct rebind
      {
        typedef blocked_tbb_allocator<U,size_type,block_size> other;
      };

    };

    template<typename T, typename size_t, size_t bs>
    struct allocator_domain<
      blocked_tbb_allocator<T,size_t,bs>
      >
    {
      typedef Domain::Host type;
    };

#endif

  } // namespace Memory
} //namespace Dune

#endif // DUNE_COMMON_MEMORY_BLOCKED_ALLOCATOR_HH
