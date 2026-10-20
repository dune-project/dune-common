// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_ALIGNED_ALLOCATOR_HH
#define DUNE_ALIGNED_ALLOCATOR_HH

#include <algorithm>
#include <bit>
#include <cstddef>
#include <limits>
#include <new>

namespace Dune
{

  /**
     @ingroup Allocators
     @brief Allocators which guarantee a minimum alignment of the memory.
     @details This allocator provides a simple interface to allocate memory with a
     minimum alignment. The actual alignment of the allocated memory is the
     maximum of the minimum alignment and the alignment of the type T. This
     ensures that the allocated memory is always aligned to the requirements of
     the type T, while also allowing for bigger alignments when needed. Note that
     the minimum alignment is also guaranteed in rebinding, i.e. when allocating
     memory for a different type U.

     @tparam T            Type of the object one wants to allocate
     @tparam MinAlignment Minimum alignment of the allocated memory.
  */
  template<class T, std::size_t MinAlignment = 1>
  class AlignedAllocator
  {

  public:

    //! Rebind allocator to type U
    template<class U>
    struct rebind
    {
      using other = AlignedAllocator<U, MinAlignment>;
    };

    //! Type of the object to allocate
    using value_type = T;

    static_assert(std::has_single_bit(MinAlignment), "MinAlignment must be a power of 2");

    //! Alignment of the allocated memory
    static constexpr std::size_t alignment = std::max(MinAlignment, std::alignment_of<T>::value);

    //! Default constructor
    AlignedAllocator() noexcept = default;

    //! Construct an aligned allocator from another aligned allocator of different type
    template<class U>
    AlignedAllocator(const AlignedAllocator<U, MinAlignment>&) noexcept
    {
    }

    /**
     * @brief Allocate n objects of type T
     * @param n Number of objects to allocate
     * @return Pointer to the allocated memory
     */
    T* allocate(std::size_t n)
    {
      if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
        throw std::bad_array_new_length();
      return static_cast<T*>(
        ::operator new(n * sizeof(T), std::align_val_t{ alignment }));
    }

    /**
     * @brief Deallocate memory previously allocated with allocate()
     * @param p Pointer to the memory to deallocate
     * @param n Number of objects to deallocate
     */
    void deallocate(T* p, std::size_t n)
    {
      ::operator delete(p, n * sizeof(T), std::align_val_t{ alignment });
    }
  };

  //! Compare two aligned allocators of different types for equality
  template <class T, class U, std::size_t A>
  bool operator==(const AlignedAllocator<T, A>&, const AlignedAllocator<U, A>&) noexcept { return true; }

} // namespace Dune

#endif // DUNE_ALIGNED_ALLOCATOR_HH
