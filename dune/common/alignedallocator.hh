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
     @brief Allocators which guarantees alignement of the memory

     @tparam T          type of the object one wants to allocate
     @tparam Alignement explicitly specify the alignement, by default it is std::alignment_of<T>::value
   */
  template<class T, int Alignment = -1>
  class AlignedAllocator : public MallocAllocator<T> {
  public:
    using pointer = typename MallocAllocator<T>::pointer;
    using size_type = typename MallocAllocator<T>::size_type;
    template <class U> struct rebind {
      typedef AlignedAllocator<U,Alignment> other;
    };
    //! allocate n objects of type T
    pointer allocate(size_type n, const void* hint = 0)
    {
      int alignment = (Alignment==-1) ? std::alignment_of<T>::value : Alignment;

      DUNE_UNUSED_PARAMETER(hint);
      if (n > this->max_size())
        throw std::bad_alloc();

      pointer ret =
        static_cast<pointer>(aligned_alloc(alignment, n * sizeof(T)));
      if (!ret)
        throw std::bad_alloc();

      return ret;
    }
  };

}

#endif // DUNE_ALIGNED_ALLOCATOR_HH
