// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_POOLALLOCATOR_HH
#define DUNE_COMMON_POOLALLOCATOR_HH

/** \file
 * \brief An stl-compliant pool allocator
 */

#include <numeric>
#include <typeinfo>
#include <iostream>
#include <cassert>
#include <new>

#ifndef DOXYGEN
// forward declarations.
// we need to know the test function to declare it friend
template<std::size_t size, typename T>
struct testPoolMain;
#endif

namespace Dune
{

  template<typename T, std::size_t s>
  class Pool;

  template<typename T, std::size_t s>
  class PoolAllocator;

}

namespace std
{
  /*
     template<class T, std::size_t S>
     inline ostream& operator<<(ostream& os, Dune::Pool<T,S>& pool)
     {
     os<<"pool="<<&pool<<" allocated_="<<pool.allocated_;
     return os;
     }

     template<class T, std::size_t S>
     inline ostream& operator<<(ostream& os, Dune::PoolAllocator<T,S>& pool)
     {
     os<<pool.memoryPool_<<std::endl;
     return os;
     }
   */
}


namespace Dune
{
  /**
   * @file
   * This file implements the classes Pool and PoolAllocator providing
   * memory allocation for objects in chunks.
   * @author Markus Blatt
   */
  /**
   * @addtogroup Allocators
   *
   * @{
   */

  /**
   * @brief A memory pool of objects.
   *
   * The memory for the objects is organized in chunks.
   * Each chunks is capable of holding a specified number of
   * objects. The allocated objects will be properly aligned
   * for fast access.
   * Deallocated objects are cached for reuse to prevent memory
   * fragmentation.
   * @warning If the size of the objects allocated is less than the
   * size of a pointer memory is wasted.
   * @warning Due to aligned issues at the number of bytes of the
   * alignment prerequisite (< 4 bytes) are wasted. This effect
   * becomes negligible for big sizes of chunkSize.
   *
   * \tparam T The type that is allocated by us.
   * \tparam s The size of a memory chunk in bytes.
   */
  template<class T, std::size_t s>
  class Pool
  {
    // make the test function friend
    friend struct ::testPoolMain<s,T>;

    //friend std::ostream& std::operator<<<>(std::ostream&,Pool<T,s>&);
    template< class, std::size_t > friend class PoolAllocator;

  private:

    /** @brief Reference to next free element. */
    struct Reference
    {
      Reference *next_;
    };

  public:

    /** @brief The type of object we allocate memory for. */
    typedef T MemberType;

    /**
     * @brief The size of a union of Reference and MemberType.
     */
    constexpr static int unionSize = (sizeof(MemberType) < sizeof(Reference)) ?
                                      sizeof(Reference) : sizeof(MemberType);

    /**
     * @brief Size requirement. At least one object has to
     * stored.
     */
    constexpr static int size = (sizeof(MemberType) <= s && sizeof(Reference) <= s) ?
                                 s : unionSize;

    /**
     * @brief The alignment that suits both the MemberType and
     * the Reference (i.e. their least common multiple).
     */
    constexpr static int alignment = std::lcm(alignof(MemberType), alignof(Reference));

    /**
     * @brief The aligned size of the type.
     *
     * This size is bigger than sizeof of the type and a multiple of
     * the alignment requirement.
     */
    constexpr static int alignedSize = (unionSize % alignment == 0) ?
                                        unionSize :
                                        ((unionSize / alignment + 1) * alignment);

    /**
     * @brief The size of each chunk memory chunk.
     *
     * Will be adapted to be a multiple of the alignment
     */
    constexpr static int chunkSize = (size % alignment == 0) ?
                                      size : ((size / alignment + 1)* alignment);

    /**
     * @brief The number of element each chunk can hold.
     */
    constexpr static int elements = (chunkSize / alignedSize);

  private:
    /** @brief Chunk of memory managed by the pool. */
    struct Chunk
    {

      //friend int testPool<s,T>();

      /** @brief The memory we hold. */
      alignas(alignment) char chunk_[chunkSize];

      /** @brief The next element */
      Chunk *next_;
    };

  public:
    /** @brief Constructor. */
    inline Pool();
    /** @brief Destructor. */
    inline ~Pool();
    /**
     * @brief Get a new or recycled object
     * @return A pointer to the object memory.
     */
    inline void* allocate();
    /**
     * @brief Free an object.
     * @param o The pointer to memory block of the object.
     */
    inline void free(void* o);

    /**
     * @brief Print elements in pool for debugging.
     */
    inline void print(std::ostream& os);

  private:

    // Prevent Copying!
    Pool(const Pool<MemberType,s>&);

    void operator=(const Pool<MemberType,s>& pool) const;
    /** @brief Grow our pool.*/
    inline void grow();
    /** @brief The first free element. */
    Reference *head_;
    /** @brief Our memory chunks. */
    Chunk *chunks_;
    /* @brief The number of currently allocated elements. */
    //size_t allocated_;

  };

  /**
   * @brief An allocator managing a pool of objects for reuse.
   *
   * This allocator is specifically useful for small data types
   * where new and delete are too expensive.
   *
   * It uses a pool of memory chunks where the objects will be allocated.
   * This means that assuming that N objects fit into memory only every N-th
   * request for an object will result in memory allocation.
   *
   * @warning It is not suitable
   * for the use in standard containers as it cannot allocate
   * arrays of arbitrary size
   *
   * \tparam T The type that will be allocated.
   * \tparam s The number of elements to fit into one memory chunk.
   */
  template<class T, std::size_t s>
  class PoolAllocator
  {
    //friend std::ostream& std::operator<<<>(std::ostream&,PoolAllocator<T,s>&);

  public:
    /**
     * @brief Type of the values we construct and allocate.
     */
    typedef T value_type;

    /**
     * @brief The number of objects to fit into one memory chunk
     * allocated.
     */
    constexpr static int size = s * sizeof(value_type);

    /**
     * @brief The pointer type.
     */
    typedef T* pointer;

    /**
     * @brief The constant pointer type.
     */
    typedef const T* const_pointer;

    /**
     * @brief The reference type.
     */
    typedef T& reference;

    /**
     * @brief The constant reference type.
     */
    typedef const T& const_reference;

    /**
     * @brief The size type.
     */
    typedef std::size_t size_type;

    /**
     * @brief The difference_type.
     */
    typedef std::ptrdiff_t difference_type;

    /**
     * @brief Constructor.
     */
    inline PoolAllocator();

    /**
     * @brief Copy Constructor that does not copy the memory pool.
     */
    template<typename U, std::size_t u>
    inline PoolAllocator(const PoolAllocator<U,u>&)
    {
      // we allow copying but never copy the pool
      // to have a clear ownership of allocated pointers.
    }

    /// \brief Copy constructor that does not copy the memory pool.
    PoolAllocator(const PoolAllocator&)
    {
      // we allow copying but never copy the pool
      // to have a clear ownership of allocated pointers.
      // For this behaviour we have to implement
      // the copy constructor, because the default
      // one would copy the pool and deallocation
      // of it would break.
    }
    /**
     * @brief Allocates objects.
     * @param n The number of objects to allocate. Has to be one!
     * @param hint Ignored hint.
     * @return A pointer tp the allocated elements.
     */
    inline pointer allocate(std::size_t n, const_pointer hint=0);

    /**
     * @brief Free objects.
     *
     * Does not call the destructor!
     * @param n The number of objects to free. Has to be one!
     * @param p Pointer to the first object.
     */
    inline void deallocate(pointer p, std::size_t n);

    /**
     * @brief Construct an object.
     * @param p Pointer to the object.
     * @param value The value to initialize it to.
     */
    inline void construct(pointer p, const_reference value);

    /**
     * @brief Destroy an object without freeing memory.
     * @param p Pointer to the object.
     */
    inline void destroy(pointer p);

    /**
     * @brief Convert a reference to a pointer.
     */
    inline pointer  address(reference x) const { return &x; }


    /**
     * @brief Convert a reference to a pointer.
     */
    inline const_pointer address(const_reference x) const { return &x; }

    /**
     * @brief Not correctly implemented, yet!
     */
    inline int max_size() const noexcept { return 1; }

    /**
     * @brief Rebind the allocator to another type.
     */
    template<class U>
    struct rebind
    {
      typedef PoolAllocator<U,s> other;
    };

    /** @brief The type of the memory pool we use. */
    typedef Pool<T,size> PoolType;

  private:
    /**
     * @brief The underlying memory pool.
     */
    PoolType memoryPool_;
  };

  // specialization for void
  template <std::size_t s>
  class PoolAllocator<void,s>
  {
  public:
    typedef void*       pointer;
    typedef const void* const_pointer;
    // reference to void members are impossible.
    typedef void value_type;
    template <class U> struct rebind
    {
      typedef PoolAllocator<U,s> other;
    };
  };


  template<typename T1, std::size_t t1, typename T2, std::size_t t2>
  bool operator==(const PoolAllocator<T1,t1>&, const PoolAllocator<T2,t2>&)
  {
    return false;
  }


  template<typename T1, std::size_t t1, typename T2, std::size_t t2>
  bool operator!=(const PoolAllocator<T1,t1>&, const PoolAllocator<T2,t2>&)
  {
    return true;
  }

  template<typename T, std::size_t t1, std::size_t t2>
  bool operator==(const PoolAllocator<T,t1>& p1, const PoolAllocator<T,t2>& p2)
  {
    return &p1==&p2;
  }


  template<typename T, std::size_t t1, std::size_t t2>
  bool operator!=(const PoolAllocator<T,t1>& p1, const PoolAllocator<T,t2>& p2)
  {
    return &p1 != &p2;
  }

  template<typename T, std::size_t t1, std::size_t t2>
  bool operator==(const PoolAllocator<void,t1>&, const PoolAllocator<T,t2>&)
  {
    return false;
  }


  template<typename T, std::size_t t1, std::size_t t2>
  bool operator!=(const PoolAllocator<void,t1>&, const PoolAllocator<T,t2>&)
  {
    return true;
  }

  template<std::size_t t1, std::size_t t2>
  bool operator==(const PoolAllocator<void,t1>& p1, const PoolAllocator<void,t2>& p2)
  {
    return &p1==&p2;
  }

  template<std::size_t t1, std::size_t t2>
  bool operator!=(const PoolAllocator<void,t1>& p1, const PoolAllocator<void,t2>& p2)
  {
    return &p1!=&p2;
  }

  template<class T, std::size_t S>
  inline Pool<T,S>::Pool()
    : head_(0), chunks_(0) //, allocated_(0)
  {
    static_assert(sizeof(T)<=unionSize, "Library Error: type T is too big");
    static_assert(sizeof(Reference)<=unionSize, "Library Error: type of reference is too big");
    static_assert(unionSize<=alignedSize, "Library Error: alignedSize too small");
    static_assert(sizeof(T)<=chunkSize, "Library Error: chunkSize must be able to hold at least one value");
    static_assert(sizeof(Reference)<=chunkSize, "Library Error: chunkSize must be able to hold at least one reference");
    static_assert(chunkSize % alignment == 0, "Library Error: compiler cannot calculate!");
    static_assert(elements>=1, "Library Error: we need to hold at least one element!");
    static_assert(elements*alignedSize<=chunkSize, "Library Error: aligned elements must fit into chuck!");
  }

  template<class T, std::size_t S>
  inline Pool<T,S>::~Pool()
  {
    /*
       if(allocated_!=0)
       std::cerr<<"There are still "<<allocated_<<" allocated elements by the Pool<"<<typeid(T).name()<<","<<S<<"> "
               <<static_cast<void*>(this)<<"! This is a memory leak and might result in segfaults"
               <<std::endl;
     */
    // delete the allocated chunks.
    Chunk *current=chunks_;

    while(current!=0)
    {
      Chunk *tmp = current;
      current = current->next_;
      delete tmp;
    }
  }

  template<class T, std::size_t S>
  inline void Pool<T,S>::print(std::ostream& os)
  {
    Chunk* current=chunks_;
    while(current) {
      os<<current<<" ";
      current=current->next_;
    }
    os<<current<<" ";
  }

  template<class T, std::size_t S>
  inline void Pool<T,S>::grow()
  {
    Chunk *newChunk = new Chunk;
    newChunk->next_ = chunks_;
    chunks_ = newChunk;

    char* start = chunks_->chunk_;
    char* last  = &start[elements*alignedSize];
    Reference* ref = new (start) (Reference);

    // grow is only called if head==0,
    assert(!head_);

    head_ = ref;

    for(char* element=start+alignedSize; element<last; element=element+alignedSize) {
      Reference* next = new (element) (Reference);
      ref->next_ = next;
      ref = next;
    }
    ref->next_=0;
  }

  template<class T, std::size_t S>
  inline void Pool<T,S>::free(void* b)
  {
    if(b) {
#ifndef NDEBUG
      Chunk* current=chunks_;
      while(current) {
        if(static_cast<void*>(current->chunk_)<=b &&
           static_cast<void*>(current->chunk_+chunkSize)>b)
          break;
        current=current->next_;
      }
      if(!current)
        throw std::bad_alloc();
#endif
      Reference* freed = static_cast<Reference*>(b);
      freed->next_ = head_;
      head_ = freed;
      //--allocated_;
    }
    else
    {
      std::cerr<< "Tried to free null pointer! "<<b<<std::endl;
      throw std::bad_alloc();
    }
  }

  template<class T, std::size_t S>
  inline void* Pool<T,S>::allocate()
  {
    if(!head_)
      grow();

    Reference* p = head_;
    head_ = p->next_;
    //++allocated_;
    return p;
  }

  template<class T, std::size_t s>
  inline PoolAllocator<T,s>::PoolAllocator()
  { }

  template<class T, std::size_t s>
  inline typename PoolAllocator<T,s>::pointer
  PoolAllocator<T,s>::allocate(std::size_t n, const_pointer)
  {
    if(n==1)
      return static_cast<T*>(memoryPool_.allocate());
    else
      throw std::bad_alloc();
  }

  template<class T, std::size_t s>
  inline void PoolAllocator<T,s>::deallocate(pointer p, std::size_t n)
  {
    for(size_t i=0; i<n; i++)
      memoryPool_.free(p++);
  }

  template<class T, std::size_t s>
  inline void PoolAllocator<T,s>::construct(pointer p, const_reference value)
  {
    ::new (static_cast<void*>(p))T(value);
  }

  template<class T, std::size_t s>
  inline void PoolAllocator<T,s>::destroy(pointer p)
  {
    p->~T();
  }

  /** @} */
}
#endif
