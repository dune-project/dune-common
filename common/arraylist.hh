// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_ARRAYLIST_HH
#define DUNE_ARRAYLIST_HH

#include <vector>
#include "smartpointer.hh"
#include "fixedarray.hh"
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
   * Internally the data is organized in a list of arrays of fixed size.
   * Whenever the capacity of the array list is not sufficient a new
   * Dune::FixedArray is allocated. In contrast to
   * std::vector this approach prevents data copying. On the outside
   * we provide the same interface as the stl random access containers.
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

    enum
    {
      /**
       * @brief The number of elements in one chunk of the list.
       * This has to be at least one. The default is 100.
       */
      chunkSize_ = (N > 0) ? N : 1
    };

    /**
     * @brief A random access iterator.
     */
    typedef ArrayListIterator<MemberType,N,A> iterator;

    /**
     * @brief A constant random access iterator.
     */
    typedef ConstArrayListIterator<MemberType,N,A> const_iterator;


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
    inline void push_back(const T& entry);

    /**
     * @brief Get the element at specific position.
     * @param i The index of the position.
     * @return The element at that position.
     */
    inline T& operator[](int i);

    /**
     * @brief Get the element at specific position.
     * @param i The index of the position.
     * @return The element at that position.
     */
    inline const T& operator[](int i) const;

    /**
     * @brief Get the number of elements in the lisz.
     * @return The number of elements.
     */
    inline int size() const;

    /**
     * @brief Get the current capacity of the list.
     * @return The capacity.
     */
    inline int capacity() const;

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
     * @brief The allocators for the fixed array.
     */
    typedef typename A::template rebind<SmartPointer<FixedArray<MemberType,chunkSize_> > >::other
    SmartPointerAllocator;

    /**
     * @brief The iterator needs access to the private variables.
     */
    friend class ArrayListIterator<T,N,A>;
    friend class ConstArrayListIterator<T,N,A>;

    /** @brief the data chunks of our list. */
    std::vector<SmartPointer<FixedArray<MemberType,chunkSize_> >,
        SmartPointerAllocator> chunks_;
    /** @brief The current data capacity. */
    int capacity_;
    /** @brief The current number of elements in our data structure. */
    int size_;
    /** @brief The index of the first entry. */
    int start_;
    /**
     * @brief Get the element at specific position.
     *
     * Index 0 always refers to the first entry in the list
     * whether it is erased or not!
     * @param i The index of the position.
     * @return The element at that position.
     */
    inline T& elementAt(int i);

    /**
     * @brief Get the element at specific position.
     *
     * Index 0 always refers to the first entry in the list
     * whether it is erased or not!
     * @param i The index of the position.
     * @return The element at that position.
     */
    inline const T& elementAt(int i) const;
  };


  /**
   * @brief A random access iterator for the Dune::ArrayList class.
   */
  template<class T, int N, class A>
  class ArrayListIterator : public RandomAccessIteratorFacade<ArrayListIterator<T,N,A>,T>
  {

    friend class ArrayList<T,N,A>;
    friend class ConstArrayListIterator<T,N,A>;
  public:
    /**
     * @brief The member type.
     */
    typedef T MemberType;


    enum
    {
      /**
       * @brief The number of elements in one chunk of the list.
       *
       * This has to be at least one. The default is 100.
       */
      chunkSize_ = (N > 0) ? N : 1
    };


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
     * @return The value at that postion.
     */
    inline MemberType& elementAt(int i) const;

    /**
     * @brief Access the element at the current position.
     * @return The element at the current position.
     */
    inline MemberType& dereference() const;

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
    inline int position(){return position_;}

    /** \todo Please doc me! */
    inline void advance(int n);

    /** \todo Please doc me! */
    inline int distanceTo(const ArrayListIterator<MemberType,N,A>& other) const;

    /** \todo Please doc me! */
    inline ArrayListIterator<MemberType,N,A>& operator=(const ArrayListIterator<MemberType,N,A>& other);

    //! Standard constructor
    inline ArrayListIterator() : position_(0)
    {}

  private:
    /**
     * @brief Constructor.
     * @param list The list we are an iterator for.
     * @param position The initial position of the iterator.
     */
    inline ArrayListIterator(ArrayList<T,N,A>& arrayList, int position);

    /**
     * @brief The current postion.
     */
    int position_;
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
    : public RandomAccessIteratorFacade<ConstArrayListIterator<T,N,A>, const T>
  {

    friend class ArrayList<T,N,A>;
    friend class ArrayListIterator<T,N,A>;

  public:
    /**
     * @brief The member type.
     */
    typedef T MemberType;


    enum
    {
      /**
       * @brief The number of elements in one chunk of the list.
       *
       * This has to be at least one. The default is 100.
       */
      chunkSize_ = (N > 0) ? N : 1
    };

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
    inline void advance(int n);

    /** \todo Please doc me! */
    inline int distanceTo(const ConstArrayListIterator<MemberType,N,A>& other) const;

    /**
     * @brief Get the value of the list at an arbitrary position.
     * @return The value at that postion.
     */
    inline const MemberType& elementAt(int i) const;

    /**
     * @brief Access the element at the current position.
     * @return The element at the current position.
     */
    inline const MemberType& dereference() const;

    inline const ConstArrayListIterator<MemberType,N,A>& operator=(const ConstArrayListIterator<MemberType,N,A>& other);

    inline ConstArrayListIterator() : position_(0)
    {}

    inline ConstArrayListIterator(const ArrayListIterator<MemberType,N,A>& other);

  private:

    /**
     * @brief Constructor.
     * @param list The list we are an iterator for.
     * @param position The initial position of the iterator.
     */
    inline ConstArrayListIterator(const ArrayList<T,N,A>& arrayList, int position);

    /**
     * @brief The current postion.
     */
    int position_;
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
  int ArrayList<T,N,A>::size() const
  {
    return size_;
  }

  template<class T, int N, class A>
  int ArrayList<T,N,A>::capacity() const
  {
    return capacity_;
  }

  template<class T, int N, class A>
  void ArrayList<T,N,A>::push_back(const T& entry)
  {
    int index=start_+size_;
    if(index==capacity_)
    {
      chunks_.push_back(SmartPointer<FixedArray<MemberType,chunkSize_> >());
      capacity_ += chunkSize_;
    }
    elementAt(index)=entry;
    ++size_;
  }

  template<class T, int N, class A>
  T& ArrayList<T,N,A>::operator[](int i)
  {
    return elementAt(start_+i);
  }


  template<class T, int N, class A>
  const T& ArrayList<T,N,A>::operator[](int i) const
  {
    return elementAt(start_+i);
  }

  template<class T, int N, class A>
  T& ArrayList<T,N,A>::elementAt(int i)
  {
    return chunks_[i/chunkSize_]->operator[](i%chunkSize_);
  }


  template<class T, int N, class A>
  const T& ArrayList<T,N,A>::elementAt(int i) const
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
    int distance = start_/chunkSize_;
    if(distance>0) {
      // Number of chunks with entries in it;
      int chunks = ((start_%chunkSize_ + size_)/chunkSize_ );

      typedef typename std::vector<SmartPointer<FixedArray<MemberType,
                  chunkSize_> > >::iterator iterator;

      // Copy chunks to the left.
      std::copy(chunks_.begin()+distance,
                chunks_.begin()+(distance+chunks), chunks_.begin());

      // Calculate new parameters
      start_ = start_ % chunkSize_;
      //capacity += distance * chunkSize_;
    }
  }

  template<class T, int N, class A>
  void ArrayListIterator<T,N,A>::advance(int i)
  {
    position_+=i;
  }

  template<class T, int N, class A>
  void ConstArrayListIterator<T,N,A>::advance(int i)
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
  T& ArrayListIterator<T,N,A>::elementAt(int i) const
  {
    i+=position_;
    return list_->elementAt(i+position_);
  }

  template<class T, int N, class A>
  const T& ConstArrayListIterator<T,N,A>::elementAt(int i) const
  {
    return list_->elementAt(i+position_);
  }

  template<class T, int N, class A>
  T& ArrayListIterator<T,N,A>::dereference() const
  {
    return list_->elementAt(position_);
  }

  template<class T, int N, class A>
  const T& ConstArrayListIterator<T,N,A>::dereference() const
  {
    return list_->elementAt(position_);
  }

  template<class T, int N, class A>
  int ArrayListIterator<T,N,A>::distanceTo(const ArrayListIterator<T,N,A>& other) const
  {
    // Makes only sense if we reference a common list
    assert(list_==(other.list_));
    return other.position_ - position_;
  }

  template<class T, int N, class A>
  int ConstArrayListIterator<T,N,A>::distanceTo(const ConstArrayListIterator<T,N,A>& other) const
  {
    // Makes only sense if we reference a common list
    assert(list_==(other.list_));
    return other.position_ - position_;
  }

  template<class T, int N, class A>
  ArrayListIterator<T,N,A>& ArrayListIterator<T,N,A>::operator=(const ArrayListIterator<T,N,A>& other)
  {
    position_=other.position_;
    list_=other.list_;
    return *this;
  }

  template<class T, int N, class A>
  const ConstArrayListIterator<T,N,A>& ConstArrayListIterator<T,N,A>::operator=(const ConstArrayListIterator<T,N,A>& other)
  {
    position_=other.position_;
    list_=other.list_;
    return *this;
  }

  template<class T, int N, class A>
  void ArrayListIterator<T,N,A>::eraseToHere()
  {
    list_->size_ -= ++position_ - list_->start_;
    // chunk number of the new position.
    int posChunkStart = position_ / chunkSize_;
    // number of chunks to deallocate
    int chunks = (position_ - list_->start_ + list_->start_ % chunkSize_)
                 / chunkSize_;
    list_->start_ = position_;

    // Deallocate memory not needed any more.
    for(int chunk=0; chunk<chunks; chunk++)
      list_->chunks_[--posChunkStart].deallocate();

    // As new entries only get append the capacity shrinks
    list_->capacity_-=chunks*chunkSize_;
  }

  template<class T, int N, class A>
  ArrayListIterator<T,N,A>::ArrayListIterator(ArrayList<T,N,A>& arrayList, int position)
    : position_(position), list_(&arrayList)
  {}


  template<class T, int N, class A>
  ConstArrayListIterator<T,N,A>::ConstArrayListIterator(const ArrayList<T,N,A>& arrayList, int position)
    : position_(position), list_(&arrayList)
  {}

  template<class T, int N, class A>
  ConstArrayListIterator<T,N,A>::ConstArrayListIterator(const ArrayListIterator<T,N,A>& other)
    : position_(other.position_), list_(other.list_)
  {}


  /** @} */
}
#endif
