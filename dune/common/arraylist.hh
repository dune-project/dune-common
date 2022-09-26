// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#ifndef DUNE_COMMON_ARRAYLIST_HH
#define DUNE_COMMON_ARRAYLIST_HH

#include <array>
#include <cassert>
#include <memory>
#include <vector>
#include "iteratorfacades.hh"

namespace Dune
{
  // forward declaration
  template<class T, int N, class A>
  class ArrayListIterator;

  template<class T, int N, class A>
  class ConstArrayListIterator;

  /**
   * @file
   * \brief Implements a random-access container that can efficiently change size (similar to std::deque)
   *
   * This file implements the class ArrayList which behaves like
   * dynamically growing array together with
   * the class ArrayListIterator which is random access iterator as needed
   * by the stl for sorting and other algorithms.
   * @author Markus Blatt
   */
  /**
   * @addtogroup Common
   *
   * @{
   */

  /**
   * @brief A dynamically growing  random access list.
   *
   * Internally the data is organised in a list of arrays of fixed size.
   * Whenever the capacity of the array list is not sufficient a new
   * std::array is allocated. In contrast to
   * std::vector this approach prevents data copying. On the outside
   * we provide the same interface as the stl random access containers.
   *
   * While the concept sounds quite similar to std::deque there are slight
   * but crucial differences:
   * - In contrast to std:deque the actual implementation (a list of arrays)
   * is known. While
   * for std::deque there are at least two possible implementations
   * (dynamic array or using a double linked list.
   * - In contrast to std:deque there is not insert which invalidates iterators
   * but our push_back method leaves all iterators valid.
   * - Additional functionality lets one delete entries before and at an
   * iterator while moving the iterator to the next valid position.
   */
  template<class T, int N=100, class A=std::allocator<T> >
  class ArrayList
  {
  public:
    /**
     * @brief The member type that is stored.
     *
     * Has to be assignable and has to have an empty constructor.
     */
    typedef T MemberType;

    /**
     * @brief Value type for stl compliance.
     */
    typedef T value_type;

    /**
     * @brief The type of a reference to the type we store.
     */
    typedef T& reference;

    /**
     * @brief The type of a const reference to the type we store.
     */
    typedef const T& const_reference;

    /**
     * @brief The type of a pointer to the type we store.
     */
    typedef T* pointer;

    /**
     * @brief The type of a const pointer to the type we store.
     */
    typedef const T* const_pointer;

    /**
     * @brief The number of elements in one chunk of the list.
     * This has to be at least one. The default is 100.
     */
    constexpr static int chunkSize_ = (N > 0) ? N : 1;

    /**
     * @brief A random access iterator.
     */
    typedef ArrayListIterator<MemberType,N,A> iterator;

    /**
     * @brief A constant random access iterator.
     */
    typedef ConstArrayListIterator<MemberType,N,A> const_iterator;

    /**
     * @brief The size type.
     */
    typedef std::size_t size_type;

    /**
     * @brief The difference type.
     */
    typedef std::ptrdiff_t difference_type;

    /**
     * @brief Get an iterator that is positioned at the first element.
     * @return The iterator.
     */
    iterator begin();

    /**
     * @brief Get a random access iterator that is positioned at the
     * first element.
     * @return The iterator.
     */
    const_iterator begin() const;

    /**
     * @brief Get a random access iterator positioned after the last
     * element
     */
    iterator end();

    /**
     * @brief Get a random access iterator positioned after the last
     * element
     */
    const_iterator end() const;

    /**
     * @brief Append an entry to the list.
     * @param entry The new entry.
     */
    inline void push_back(const_reference entry);

    /**
     * @brief Get the element at specific position.
     * @param i The index of the position.
     * @return The element at that position.
     */
    inline reference operator[](size_type i);

    /**
     * @brief Get the element at specific position.
     * @param i The index of the position.
     * @return The element at that position.
     */
    inline const_reference operator[](size_type i) const;

    /**
     * @brief Get the number of elements in the list.
     * @return The number of elements.
     */
    inline size_type size() const;

    /**
     * @brief Purge the list.
     *
     * If there are empty chunks at the front all nonempty
     * chunks will be moved towards the front and the capacity
     * increases.
     */
    inline void purge();

    /**
     * @brief Delete all entries from the list.
     */
    inline void clear();
    /**
     * @brief Constructs an Array list with one chunk.
     */
    ArrayList();

  private:

    /**
     * @brief The allocators for the smart pointer.
     */
    using SmartPointerAllocator = typename std::allocator_traits<A>::template rebind_alloc< std::shared_ptr< std::array<MemberType,chunkSize_> > >;

    /**
     * @brief The allocator for the fixed array.
     */
    using ArrayAllocator = typename std::allocator_traits<A>::template rebind_alloc< std::array<MemberType,chunkSize_> >;

    /**
     * @brief The iterator needs access to the private variables.
     */
    friend class ArrayListIterator<T,N,A>;
    friend class ConstArrayListIterator<T,N,A>;

    /** @brief the data chunks of our list. */
    std::vector<std::shared_ptr<std::array<MemberType,chunkSize_> >,
        SmartPointerAllocator> chunks_;
    /** @brief The current data capacity.
     * This is the capacity that the list could have theoretically
     * with this number of chunks. That is chunks * chunkSize.
     * In practice some of the chunks at the beginning might be empty
     * (i.e. null pointers in the first start_/chunkSize chunks)
     * because of previous calls to eraseToHere.
     * start_+size_<=capacity_ holds.
     */
    size_type capacity_;
    /** @brief The current number of elements in our data structure. */
    size_type size_;
    /** @brief The index of the first entry. */
    size_type start_;
    /**
     * @brief Get the element at specific position.
     *
     * Index 0 always refers to the first entry in the list
     * whether it is erased or not!
     * @param i The index of the position.
     * @return The element at that position.
     */
    inline reference elementAt(size_type i);

    /**
     * @brief Get the element at specific position.
     *
     * Index 0 always refers to the first entry in the list
     * whether it is erased or not!
     * @param i The index of the position.
     * @return The element at that position.
     */
    inline const_reference elementAt(size_type i) const;
  };


  /**
   * @brief A random access iterator for the Dune::ArrayList class.
   */
  template<class T, int N, class A>
  class ArrayListIterator : public RandomAccessIteratorFacade<ArrayListIterator<T,N,A>,
                                typename A::value_type,
                                typename A::value_type &,
                                typename A::difference_type>
  {

    friend class ArrayList<T,N,A>;
    friend class ConstArrayListIterator<T,N,A>;
  public:
    /**
     * @brief The member type.
     */
    typedef typename A::value_type MemberType;

    typedef typename A::difference_type difference_type;

    typedef typename A::size_type size_type;

    using reference = typename A::value_type &;

    using const_reference = typename A::value_type const&;

    /**
     * @brief The number of elements in one chunk of the list.
     *
     * This has to be at least one. The default is 100.
     */
    constexpr static int chunkSize_ = (N > 0) ? N : 1;


    /**
     * @brief Comares two iterators.
     * @return True if the iterators are for the same list and
     * at the position.
     */
    inline bool equals(const ArrayListIterator<MemberType,N,A>& other) const;

    /**
     * @brief Comares two iterators.
     * @return True if the iterators are for the same list and
     * at the position.
     */
    inline bool equals(const ConstArrayListIterator<MemberType,N,A>& other) const;

    /**
     * @brief Increment the iterator.
     */
    inline void increment();

    /**
     * @brief decrement the iterator.
     */
    inline void decrement();

    /**
     * @brief Get the value of the list at an arbitrary position.
     * @return The value at that position.
     */
    inline reference elementAt(size_type i) const;

    /**
     * @brief Access the element at the current position.
     * @return The element at the current position.
     */
    inline reference dereference() const;

    /**
     * @brief Erase all entries before the current position
     * and the one at the current position.
     *
     * Afterwards the iterator will be positioned at the next
     * unerased entry or the end if the list is empty.
     * This does not invalidate any iterators positioned after
     * the current position but those positioned at previous ones.
     * @return An iterator to the first position after the deleted
     * ones or to the end if the list is empty.
     */
    inline void eraseToHere();

    /** \todo Please doc me! */
    inline size_type position(){return position_;}

    /** \todo Please doc me! */
    inline void advance(difference_type n);

    /** \todo Please doc me! */
    inline difference_type distanceTo(const ArrayListIterator<T,N,A>& other) const;

    //! Standard constructor
    inline ArrayListIterator() : position_(0), list_(nullptr)
    {}

  private:
    /**
     * @brief Constructor.
     * @param list The list we are an iterator for.
     * @param position The initial position of the iterator.
     */
    inline ArrayListIterator(ArrayList<T,N,A>& arrayList, size_type position);

    /**
     * @brief The current position.
     */
    size_type position_;
    /**
     * @brief The list we are an iterator for.
     */
    ArrayList<T,N,A>* list_;
  };

  /**
   * @brief A constant random access iterator for the Dune::ArrayList class.
   */
  template<class T, int N, class A>
  class ConstArrayListIterator
    : public RandomAccessIteratorFacade<ConstArrayListIterator<T,N,A>,
          const typename A::value_type,
          typename A::value_type const&,
          typename A::difference_type>
  {

    friend class ArrayList<T,N,A>;
    friend class ArrayListIterator<T,N,A>;

  public:
    /**
     * @brief The member type.
     */
    typedef typename A::value_type MemberType;

    typedef typename A::difference_type difference_type;

    typedef typename A::size_type size_type;

    using reference = typename A::value_type &;

    using const_reference = typename A::value_type const&;

    /**
     * @brief The number of elements in one chunk of the list.
     *
     * This has to be at least one. The default is 100.
     */
    constexpr static int chunkSize_ = (N > 0) ? N : 1;

    /**
     * @brief Comares to iterators.
     * @return true if the iterators are for the same list and
     * at the position.
     */
    inline bool equals(const ConstArrayListIterator<MemberType,N,A>& other) const;

    /**
     * @brief Increment the iterator.
     */
    inline void increment();

    /**
     * @brief decrement the iterator.
     */
    inline void decrement();

    /** \todo Please doc me! */
    inline void advance(difference_type n);

    /** \todo Please doc me! */
    inline difference_type distanceTo(const ConstArrayListIterator<T,N,A>& other) const;

    /**
     * @brief Get the value of the list at an arbitrary position.
     * @return The value at that position.
     */
    inline const_reference elementAt(size_type i) const;

    /**
     * @brief Access the element at the current position.
     * @return The element at the current position.
     */
    inline const_reference dereference() const;

    inline ConstArrayListIterator() : position_(0), list_(nullptr)
    {}

    inline ConstArrayListIterator(const ArrayListIterator<T,N,A>& other);

  private:

    /**
     * @brief Constructor.
     * @param list The list we are an iterator for.
     * @param position The initial position of the iterator.
     */
    inline ConstArrayListIterator(const ArrayList<T,N,A>& arrayList, size_type position);

    /**
     * @brief The current position.
     */
    size_type position_;
    /**
     * @brief The list we are an iterator for.
     */
    const ArrayList<T,N,A>* list_;
  };


  template<class T, int N, class A>
  ArrayList<T,N,A>::ArrayList()
    : capacity_(0), size_(0), start_(0)
  {
    chunks_.reserve(100);
  }

  template<class T, int N, class A>
  void ArrayList<T,N,A>::clear(){
    capacity_=0;
    size_=0;
    start_=0;
    chunks_.clear();
  }

  template<class T, int N, class A>
  size_t ArrayList<T,N,A>::size() const
  {
    return size_;
  }

  template<class T, int N, class A>
  void ArrayList<T,N,A>::push_back(const_reference entry)
  {
    size_t index=start_+size_;
    if(index==capacity_)
    {
      chunks_.push_back(std::make_shared<std::array<MemberType,chunkSize_> >());
      capacity_ += chunkSize_;
    }
    elementAt(index)=entry;
    ++size_;
  }

  template<class T, int N, class A>
  typename ArrayList<T,N,A>::reference ArrayList<T,N,A>::operator[](size_type i)
  {
    return elementAt(start_+i);
  }


  template<class T, int N, class A>
  typename ArrayList<T,N,A>::const_reference ArrayList<T,N,A>::operator[](size_type i) const
  {
    return elementAt(start_+i);
  }

  template<class T, int N, class A>
  typename ArrayList<T,N,A>::reference ArrayList<T,N,A>::elementAt(size_type i)
  {
    return chunks_[i/chunkSize_]->operator[](i%chunkSize_);
  }


  template<class T, int N, class A>
  typename ArrayList<T,N,A>::const_reference ArrayList<T,N,A>::elementAt(size_type i) const
  {
    return chunks_[i/chunkSize_]->operator[](i%chunkSize_);
  }

  template<class T, int N, class A>
  ArrayListIterator<T,N,A> ArrayList<T,N,A>::begin()
  {
    return ArrayListIterator<T,N,A>(*this, start_);
  }

  template<class T, int N, class A>
  ConstArrayListIterator<T,N,A> ArrayList<T,N,A>::begin() const
  {
    return ConstArrayListIterator<T,N,A>(*this, start_);
  }

  template<class T, int N, class A>
  ArrayListIterator<T,N,A> ArrayList<T,N,A>::end()
  {
    return ArrayListIterator<T,N,A>(*this, start_+size_);
  }

  template<class T, int N, class A>
  ConstArrayListIterator<T,N,A> ArrayList<T,N,A>::end() const
  {
    return ConstArrayListIterator<T,N,A>(*this, start_+size_);
  }

  template<class T, int N, class A>
  void ArrayList<T,N,A>::purge()
  {
    // Distance to copy to the left.
    size_t distance = start_/chunkSize_;
    if(distance>0) {
      // Number of chunks with entries in it;
      size_t chunks = ((start_%chunkSize_ + size_)/chunkSize_ );

      // Copy chunks to the left.
      std::copy(chunks_.begin()+distance,
                chunks_.begin()+(distance+chunks), chunks_.begin());

      // Calculate new parameters
      start_ = start_ % chunkSize_;
      //capacity += distance * chunkSize_;
    }
  }

  template<class T, int N, class A>
  void ArrayListIterator<T,N,A>::advance(difference_type i)
  {
    position_+=i;
  }

  template<class T, int N, class A>
  void ConstArrayListIterator<T,N,A>::advance(difference_type i)
  {
    position_+=i;
  }


  template<class T, int N, class A>
  bool ArrayListIterator<T,N,A>::equals(const ArrayListIterator<MemberType,N,A>& other) const
  {
    // Makes only sense if we reference a common list
    assert(list_==(other.list_));
    return position_==other.position_ ;
  }


  template<class T, int N, class A>
  bool ArrayListIterator<T,N,A>::equals(const ConstArrayListIterator<MemberType,N,A>& other) const
  {
    // Makes only sense if we reference a common list
    assert(list_==(other.list_));
    return position_==other.position_ ;
  }


  template<class T, int N, class A>
  bool ConstArrayListIterator<T,N,A>::equals(const ConstArrayListIterator<MemberType,N,A>& other) const
  {
    // Makes only sense if we reference a common list
    assert(list_==(other.list_));
    return position_==other.position_ ;
  }

  template<class T, int N, class A>
  void ArrayListIterator<T,N,A>::increment()
  {
    ++position_;
  }

  template<class T, int N, class A>
  void ConstArrayListIterator<T,N,A>::increment()
  {
    ++position_;
  }

  template<class T, int N, class A>
  void ArrayListIterator<T,N,A>::decrement()
  {
    --position_;
  }

  template<class T, int N, class A>
  void ConstArrayListIterator<T,N,A>::decrement()
  {
    --position_;
  }

  template<class T, int N, class A>
  typename ArrayListIterator<T,N,A>::reference ArrayListIterator<T,N,A>::elementAt(size_type i) const
  {
    return list_->elementAt(i+position_);
  }

  template<class T, int N, class A>
  typename ConstArrayListIterator<T,N,A>::const_reference ConstArrayListIterator<T,N,A>::elementAt(size_type i) const
  {
    return list_->elementAt(i+position_);
  }

  template<class T, int N, class A>
  typename ArrayListIterator<T,N,A>::reference ArrayListIterator<T,N,A>::dereference() const
  {
    return list_->elementAt(position_);
  }

  template<class T, int N, class A>
  typename ConstArrayListIterator<T,N,A>::const_reference ConstArrayListIterator<T,N,A>::dereference() const
  {
    return list_->elementAt(position_);
  }

  template<class T, int N, class A>
  typename ArrayListIterator<T,N,A>::difference_type ArrayListIterator<T,N,A>::distanceTo(const ArrayListIterator<T,N,A>& other) const
  {
    // Makes only sense if we reference a common list
    assert(list_==(other.list_));
    return other.position_ - position_;
  }

  template<class T, int N, class A>
  typename ConstArrayListIterator<T,N,A>::difference_type ConstArrayListIterator<T,N,A>::distanceTo(const ConstArrayListIterator<T,N,A>& other) const
  {
    // Makes only sense if we reference a common list
    assert(list_==(other.list_));
    return other.position_ - position_;
  }

  template<class T, int N, class A>
  void ArrayListIterator<T,N,A>::eraseToHere()
  {
    list_->size_ -= ++position_ - list_->start_;
    // chunk number of the new position.
    size_t posChunkStart = position_ / chunkSize_;
    // number of chunks to deallocate
    size_t chunks = (position_ - list_->start_ + list_->start_ % chunkSize_)
                    / chunkSize_;
    list_->start_ = position_;

    // Deallocate memory not needed any more.
    for(size_t chunk=0; chunk<chunks; chunk++) {
      --posChunkStart;
      list_->chunks_[posChunkStart].reset();
    }

    // Capacity stays the same as the chunks before us
    // are still there. They null pointers.
    assert(list_->start_+list_->size_<=list_->capacity_);
  }

  template<class T, int N, class A>
  ArrayListIterator<T,N,A>::ArrayListIterator(ArrayList<T,N,A>& arrayList, size_type position)
    : position_(position), list_(&arrayList)
  {}


  template<class T, int N, class A>
  ConstArrayListIterator<T,N,A>::ConstArrayListIterator(const ArrayList<T,N,A>& arrayList,
                                                        size_type position)
    : position_(position), list_(&arrayList)
  {}

  template<class T, int N, class A>
  ConstArrayListIterator<T,N,A>::ConstArrayListIterator(const ArrayListIterator<T,N,A>& other)
    : position_(other.position_), list_(other.list_)
  {}


  /** @} */
}
#endif
