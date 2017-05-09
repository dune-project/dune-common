// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_MALLOC_ALLOCATOR_HH
#define DUNE_MALLOC_ALLOCATOR_HH

#include <exception>
#include <cstdlib>
#include <new>
#include <utility>
#include <dune/common/unused.hh>

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
                     const void* hint = 0)
    {
      DUNE_UNUSED_PARAMETER(hint);
      if (n > this->max_size())
        throw std::bad_alloc();

      pointer ret = static_cast<pointer>(std::malloc(n * sizeof(T)));
      if (!ret)
        throw std::bad_alloc();
      return ret;
    }

    //! deallocate n objects of type T at address p
    void deallocate(pointer p, size_type n)
    {
      DUNE_UNUSED_PARAMETER(n);
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
    template<typename ... _Args>
    void construct(pointer p, _Args&&... __args)
    {
      ::new((void *)p)T(std::forward<_Args>(__args) ...);
    }

    //! destroy an object of type T (i.e. call the destructor)
    void destroy(pointer p)
    {
      p->~T();
    }
  };
}

#endif // DUNE_MALLOC_ALLOCATOR_HH
