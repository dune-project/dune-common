// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_DEBUG_ALLOCATOR_HH
#define DUNE_DEBUG_ALLOCATOR_HH

#include <dune/common/unused.hh>
#include <exception>
#include <typeinfo>
#include <vector>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <new>
#if HAVE_SYS_MMAN_H and HAVE_MPROTECT
#include <sys/mman.h>
#else
enum DummyProtFlags { PROT_NONE, PROT_WRITE, PROT_READ };
#endif

#include "mallocallocator.hh"

namespace Dune
{

#ifndef DOXYGEN // hide implementation details from doxygen
  namespace DebugMemory
  {

    extern const std::ptrdiff_t page_size;

    struct AllocationManager
    {
      typedef std::size_t size_type;
      typedef std::ptrdiff_t difference_type;
      typedef void* pointer;

    protected:
      static void allocation_error(const char* msg);

      struct AllocationInfo;
      friend struct AllocationInfo;

#define ALLOCATION_ASSERT(A) { if (!(A))                            \
                               { allocation_error("Assertion " # A " failed");\
                               }\
};

      struct AllocationInfo
      {
        AllocationInfo(const std::type_info & t) : type(&t) {}
        const std::type_info * type;

        pointer page_ptr;
        pointer ptr;
        size_type pages;
        size_type capacity;
        size_type size;
        bool not_free;
      };

      typedef MallocAllocator<AllocationInfo> Alloc;
      typedef std::vector<AllocationInfo, Alloc> AllocationList;
      AllocationList allocation_list;

    private:
      void memprotect(void* from, difference_type len, int prot)
      {
#if HAVE_SYS_MMAN_H and HAVE_MPROTECT
        int result = mprotect(from, len, prot);
        if (result == -1)
        {

          std::cerr << "ERROR: (" << result << ": " << strerror(result) << ")" << std::endl;
          std::cerr << " Failed to ";
          if (prot == PROT_NONE)
            std::cerr << "protect ";
          else
            std::cerr << "unprotect ";
          std::cerr << "memory range: "
                    << from << ", "
                    << static_cast<void*>(
            static_cast<char*>(from) + len)
                    << std::endl;
          abort();
        }
#else
        std::cerr << "WARNING: memory protection not available" << std::endl;
#endif
      }

    public:

      ~AllocationManager ()
      {
        AllocationList::iterator it;
        bool error = false;
        for (it=allocation_list.begin(); it!=allocation_list.end(); it++)
        {
          if (it->not_free)
          {
            std::cerr << "ERROR: found memory chunk still in use: " <<
            it->capacity << " bytes at " << it->ptr << std::endl;
            error = true;
          }
          free(it->page_ptr);
        }
        if (error)
          allocation_error("lost allocations");
      }

      template<typename T>
      T* allocate(size_type n) throw(std::bad_alloc)
      {
        // setup chunk info
        AllocationInfo ai(typeid(T));
        ai.size = n;
        ai.capacity = n * sizeof(T);
        ai.pages = (ai.capacity) / page_size + 2;
        ai.not_free = true;
        size_type overlap = ai.capacity % page_size;
        int result = posix_memalign(&(ai.page_ptr), page_size, ai.pages * page_size);
        if (0 != result)
        {
          throw std::bad_alloc();
        }
        ai.ptr = static_cast<char*>(ai.page_ptr) + page_size - overlap;
        // write protect memory behind the actual data
        memprotect(static_cast<char*>(ai.page_ptr) + (ai.pages-1) * page_size,
                   page_size,
                   PROT_NONE);
        // remember the chunk
        allocation_list.push_back(ai);
        // return the ptr
        return static_cast<T*>(ai.ptr);
      }

      template<typename T>
      void deallocate(T* ptr, size_type n = 0) throw()
      {
        // compute page address
        void* page_ptr =
          static_cast<void*>(
            (char*)(ptr) - ((difference_type)(ptr) % page_size));
        // search list
        AllocationList::iterator it;
        unsigned int i = 0;
        for (it=allocation_list.begin(); it!=allocation_list.end(); it++, i++)
        {
          if (it->page_ptr == page_ptr)
          {
            // std::cout << "found memory_block in allocation " << i << std::endl;
            // sanity checks
            if (n != 0)
              ALLOCATION_ASSERT(n == it->size);
            ALLOCATION_ASSERT(ptr == it->ptr);
            ALLOCATION_ASSERT(true == it->not_free);
            ALLOCATION_ASSERT(typeid(T) == *(it->type));
            // free memory
            it->not_free = false;
#if DEBUG_ALLOCATOR_KEEP
            // write protect old memory
            memprotect(it->page_ptr,
                       (it->pages) * page_size,
                       PROT_NONE);
#else
            // unprotect old memory
            memprotect(it->page_ptr,
                       (it->pages) * page_size,
                       PROT_READ | PROT_WRITE);
            std::free(it->page_ptr);
            // remove chunk info
            allocation_list.erase(it);
#endif
            return;
          }
        }
        allocation_error("memory block not found");
      }
    };
#undef ALLOCATION_ASSERT

    extern AllocationManager alloc_man;
  }   // end namespace DebugMemory
#endif // DOXYGEN

  template<class T>
  class DebugAllocator;

  // specialize for void
  template <>
  class DebugAllocator<void> {
  public:
    typedef void* pointer;
    typedef const void* const_pointer;
    // reference to void members are impossible.
    typedef void value_type;
    template <class U> struct rebind {
      typedef DebugAllocator<U> other;
    };
  };

  // actual implementation
  /**
     @ingroup Allocators
     @brief Allocators implementation which performs different kind of memory checks

     We check:
     - access past the end
     - only free memory which was allocated with this allocator
     - list allocated memory chunks still in use upon destruction of the allocator

     When defining DEBUG_ALLOCATOR_KEEP to 1, we also check
     - double free
     - access after free

     When defining DEBUG_NEW_DELETE >= 1, we
     - overload new/delte
     - use the Debug memory management for new/delete
     - DEBUG_NEW_DELETE > 2 gives extensive debug output
   */
  template <class T>
  class DebugAllocator {
  public:
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;
    template <class U> struct rebind {
      typedef DebugAllocator<U> other;
    };

    //! create a new DebugAllocator
    DebugAllocator() throw() {}
    //! copy construct from an other DebugAllocator, possibly for a different result type
    template <class U>
    DebugAllocator(const DebugAllocator<U>&) throw() {}
    //! cleanup this allocator
    ~DebugAllocator() throw() {}

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
                     DebugAllocator<void>::const_pointer hint = 0)
    {
      DUNE_UNUSED_PARAMETER(hint);
      return DebugMemory::alloc_man.allocate<T>(n);
    }

    //! deallocate n objects of type T at address p
    void deallocate(pointer p, size_type n)
    {
      DebugMemory::alloc_man.deallocate<T>(p,n);
    }

    //! max size for allocate
    size_type max_size() const throw()
    {
      return size_type(-1) / sizeof(T);
    }

    //! copy-construct an object of type T (i.e. make a placement new on p)
    void construct(pointer p, const T& val)
    {
      ::new((void*)p)T(val);
    }
#if ( HAVE_VARIADIC_TEMPLATES && HAVE_RVALUE_REFERENCES ) || DOXYGEN
    //! construct an object of type T from variadic parameters
    //! \note works only with newer C++ compilers
    template<typename ... _Args>
    void construct(pointer p, _Args&&... __args)
    {
      ::new((void *)p)T(std::forward<_Args>(__args) ...);
    }
#endif
    //! destroy an object of type T (i.e. call the destructor)
    void destroy(pointer p)
    {
      p->~T();
    }
  };
}

#ifdef DEBUG_NEW_DELETE
void * operator new(size_t size) throw(std::bad_alloc)
{
  // try to allocate size bytes
  void *p = Dune::DebugMemory::alloc_man.allocate<char>(size);
#if DEBUG_NEW_DELETE > 2
  std::cout << "NEW " << size
            << " -> " << p
            << std::endl;
#endif
  return p;
}

void operator delete(void * p) throw()
{
#if DEBUG_NEW_DELETE > 2
  std::cout << "FREE " << p << std::endl;
#endif
  Dune::DebugMemory::alloc_man.deallocate<char>(static_cast<char*>(p));
}

#endif // DEBUG_NEW_DELETE

#endif // DUNE_DEBUG_ALLOCATOR_HH
