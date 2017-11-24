// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALIGNED_ALLOCATOR_HH
#define DUNE_ALIGNED_ALLOCATOR_HH

#include "mallocallocator.hh"
#include <cstdlib>

namespace Dune
{

  /**
     @ingroup Allocators
     @brief Allocators which guarantee alignment of the memory

     @tparam T         type of the object one wants to allocate
     @tparam Alignment explicitly specify the alignment, by default it is std::alignment_of<T>::value
   */
  template<class T, int Alignment = -1>
  class AlignedAllocator : public MallocAllocator<T> {

#if __APPLE__

    /*
     * posix_memalign() on macOS has pretty draconian restrictions on the
     * alignments that you may ask for: It has to be
     *
     * 1) a power of 2
     * 2) at least as large as sizeof(void*)
     *
     * So here is a little constexpr function that calculates just that
     * (together with the correct starting value for align fed in further down).
     */
    static constexpr int fixAlignment(int align)
    {
      return ((Alignment==-1) ? std::alignment_of<T>::value : Alignment) > align
        ? fixAlignment(align << 1) : align;
    }

#else

    /*
     * Non-Apple platforms just have to check whether an explicit alignment was
     * restricted or fall back to the default alignment of T.
     */
    static constexpr int fixAlignment(int align)
    {
      return (Alignment==-1) ? std::alignment_of<T>::value : Alignment;
    }

#endif

  public:
    using pointer = typename MallocAllocator<T>::pointer;
    using size_type = typename MallocAllocator<T>::size_type;
    template <class U> struct rebind {
      typedef AlignedAllocator<U,Alignment> other;
    };

    static constexpr int alignment = fixAlignment(sizeof(void*));

    //! allocate n objects of type T
    pointer allocate(size_type n, const void* hint = 0)
    {

      DUNE_UNUSED_PARAMETER(hint);
      if (n > this->max_size())
        throw std::bad_alloc();

#if __APPLE__
      /*
       * Apple's standard library doesn't have aligned_alloc() - C11 is still something
       * from the future in Cupertino. Luckily, they got around to finally implementing
       * posix_memalign(), so let's use that instead.
       */
      void* ret = nullptr;
      if (posix_memalign(&ret, alignment, n * sizeof(T)) != 0)
        throw std::bad_alloc();

      return static_cast<pointer>(ret);
#else
      /*
       * Everybody else gets the standard treatment.
       */
      pointer ret = static_cast<pointer>(aligned_alloc(alignment, n * sizeof(T)));
      if (!ret)
        throw std::bad_alloc();

      return ret;
#endif
    }
  };

}

#endif // DUNE_ALIGNED_ALLOCATOR_HH
