// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_MALLOC_ALLOCATOR_HH
#define DUNE_MALLOC_ALLOCATOR_HH

#include <exception>
#include <cstdlib>
#include <new>
#include <utility>

/**
 * @file
 * @brief Allocators that use malloc/free.
 */
namespace Dune
{
  /**
     @ingroup Allocators
     @brief Allocators implementation which simply calls malloc/free
   */
  template <class T>
  class MallocAllocator {
  public:
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;
    template <class U> struct rebind {
      typedef MallocAllocator<U> other;
    };

    //! create a new MallocAllocator
    MallocAllocator() noexcept {}
    //! copy construct from an other MallocAllocator, possibly for a different result type
    template <class U>
    MallocAllocator(const MallocAllocator<U>&) noexcept {}
    //! cleanup this allocator
    ~MallocAllocator() noexcept {}

    pointer address(reference x) const
    {
      return &x;
    }
    const_pointer address(const_reference x) const
    {
      return &x;
    }

    //! allocate n objects of type T
    pointer allocate(size_type n,
                     [[maybe_unused]] const void* hint = 0)
    {
      if (n > this->max_size())
        throw std::bad_alloc();

      pointer ret = static_cast<pointer>(std::malloc(n * sizeof(T)));
      if (!ret)
        throw std::bad_alloc();
      return ret;
    }

    //! deallocate n objects of type T at address p
    void deallocate(pointer p, [[maybe_unused]] size_type n)
    {
      std::free(p);
    }

    //! max size for allocate
    size_type max_size() const noexcept
    {
      return size_type(-1) / sizeof(T);
    }

    //! copy-construct an object of type T (i.e. make a placement new on p)
    void construct(pointer p, const T& val)
    {
      ::new((void*)p)T(val);
    }

    //! construct an object of type T from variadic parameters
    template<typename ... Args>
    void construct(pointer p, Args&&... args)
    {
      ::new((void *)p)T(std::forward<Args>(args) ...);
    }

    //! destroy an object of type T (i.e. call the destructor)
    void destroy(pointer p)
    {
      p->~T();
    }
  };

  //! check whether allocators are equivalent
  template<class T>
  constexpr bool
  operator==(const MallocAllocator<T> &, const MallocAllocator<T> &)
  {
    return true;
  }

  //! check whether allocators are not equivalent
  template<class T>
  constexpr bool
  operator!=(const MallocAllocator<T> &, const MallocAllocator<T> &)
  {
    return false;
  }
}

#endif // DUNE_MALLOC_ALLOCATOR_HH
