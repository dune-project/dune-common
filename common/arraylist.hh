// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef __DUNE_ARRAYLIST_HH__
#define __DUNE_ARRAYLIST_HH__

#include <vector>
#include <dune/common/smartpointer.hh>
#include <dune/common/fixedarray.hh>
#include <dune/common/iteratorfacades.hh>
namespace Dune
{
  // forward declaration
  template<class T, int N>
  class ArrayListIterator;


  template<class T, int N>
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
   * Internally the data is organized in a list of arrays of fixed size. Whenever the capacity
   * of the array list is not sufficient a new Dune::FixedArray is allocated. In contrast to
   * std::vector this approach prevents data copying. On the outside
   * we provide the same interface as the stl random access containers.
   */
  template<class T, int N=100>
  class ArrayList
  {

    /**
     * @brief The iterator needs access to the private variables.
     */
    friend class ArrayListIterator<T,N>;
    friend class ConstArrayListIterator<T,N>;

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
    typedef ArrayListIterator<MemberType,N> iterator;

    /**
     * @brief A constant random access iterator.
     */
    typedef ConstArrayListIterator<MemberType,N> const_iterator;


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
     * @brief Delete all entries from the list.
     */
    inline void clear();
    /**
     * @brief Constructs an Array list with one chunk.
     */
    ArrayList();

  protected:
    /** @brief the data chunks of our list. */
    std::vector<SmartPointer<FixedArray<MemberType,chunkSize_> > > chunks_;
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
  template<class T, int N>
  class ArrayListIterator : public RandomAccessIteratorFacade<ArrayListIterator<T,N>,T>
  {

    friend class ArrayList<T,N>;
    friend class ConstArrayListIterator<T,N>;
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
    inline bool equals(const ArrayListIterator<MemberType,N>& other) const;

    /**
     * @brief Comares two iterators.
     * @return True if the iterators are for the same list and
     * at the position.
     */
    inline bool equals(const ConstArrayListIterator<MemberType,N>& other) const;

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
     *
     * @warning This invalidates all other iterators of the list.
     * An iterator the pointed to the end of the list before
     * does not do that after this method is called.
     * @return An iterator to the first position after the deleted
     * ones or to the end if the list is empty.
     */
    inline void eraseToHere();

    inline int position(){return position_;}

    inline void advance(int n);

    inline int distanceTo(const ArrayListIterator<MemberType,N>& other) const;

    inline ArrayListIterator<MemberType,N>& operator=(const ArrayListIterator<MemberType,N>& other);

    inline ArrayListIterator() : position_(0), list_(list_)
    {}

  private:
    /**
     * @brief Constructor.
     * @param list The list we are an iterator for.
     * @param position The initial position of the iterator.
     */
    inline ArrayListIterator(ArrayList<T,N>& arrayList, int position);

    /**
     * @brief The current postion.
     */
    int position_;
    /**
     * @brief The list we are an iterator for.
     */
    ArrayList<T,N>* list_;
  };

  /**
   * @brief A constant random access iterator for the Dune::ArrayList class.
   */
  template<class T, int N>
  class ConstArrayListIterator
    : public RandomAccessIteratorFacade<ConstArrayListIterator<T,N>, const T>
  {

    friend class ArrayList<T,N>;
    friend class ArrayListIterator<T,N>;

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
    inline bool equals(const ConstArrayListIterator<MemberType,N>& other) const;

    /**
     * @brief Increment the iterator.
     */
    inline void increment();

    /**
     * @brief decrement the iterator.
     */
    inline void decrement();

    inline void advance(int n);

    inline int distanceTo(const ConstArrayListIterator<MemberType,N>& other) const;

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

    inline const ConstArrayListIterator<MemberType,N>& operator=(const ConstArrayListIterator<MemberType,N>& other);

    inline ConstArrayListIterator() : position_(0), list_(list_)
    {}

    inline ConstArrayListIterator(const ArrayListIterator<MemberType,N>& other);

  private:

    /**
     * @brief Constructor.
     * @param list The list we are an iterator for.
     * @param position The initial position of the iterator.
     */
    inline ConstArrayListIterator(const ArrayList<T,N>& arrayList, int position);

    /**
     * @brief The current postion.
     */
    int position_;
    /**
     * @brief The list we are an iterator for.
     */
    const ArrayList<T,N>* list_;
  };


  template<class T, int N>
  ArrayList<T,N>::ArrayList()
    : capacity_(0), size_(0), start_(0)
  {
    chunks_.reserve(100);
  }

  template<class T, int N>
  void ArrayList<T,N>::clear(){
    capacity_=0;
    size_=0;
    start_=0;
    chunks_.clear();
  }

  template<class T, int N>
  int ArrayList<T,N>::size() const
  {
    return size_;
  }

  template<class T, int N>
  int ArrayList<T,N>::capacity() const
  {
    return capacity_;
  }

  template<class T, int N>
  void ArrayList<T,N>::push_back(const T& entry)
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

  template<class T, int N>
  T& ArrayList<T,N>::operator[](int i)
  {
    return elementAt(start_+i);
  }


  template<class T, int N>
  const T& ArrayList<T,N>::operator[](int i) const
  {
    return elementAt(start_+i);
  }

  template<class T, int N>
  T& ArrayList<T,N>::elementAt(int i)
  {
    return chunks_[i/chunkSize_]->operator[](i%chunkSize_);
  }


  template<class T, int N>
  const T& ArrayList<T,N>::elementAt(int i) const
  {
    return chunks_[i/chunkSize_]->operator[](i%chunkSize_);
  }

  template<class T, int N>
  ArrayListIterator<T,N> ArrayList<T,N>::begin()
  {
    return ArrayListIterator<T,N>(*this, start_);
  }

  template<class T, int N>
  ConstArrayListIterator<T,N> ArrayList<T,N>::begin() const
  {
    return ConstArrayListIterator<T,N>(*this, start_);
  }

  template<class T, int N>
  ArrayListIterator<T,N> ArrayList<T,N>::end()
  {
    return ArrayListIterator<T,N>(*this, start_+size_);
  }

  template<class T, int N>
  ConstArrayListIterator<T,N> ArrayList<T,N>::end() const
  {
    return ConstArrayListIterator<T,N>(*this, start_+size_);
  }


  template<class T, int N>
  void ArrayListIterator<T,N>::advance(int i)
  {
    position_+=i;
  }

  template<class T, int N>
  void ConstArrayListIterator<T,N>::advance(int i)
  {
    position_+=i;
  }


  template<class T, int N>
  bool ArrayListIterator<T,N>::equals(const ArrayListIterator<MemberType,N>& other) const
  {
    // Makes only sense if we reference a common list
    assert(list_==(other.list_));
    return position_==other.position_ ;
  }


  template<class T, int N>
  bool ArrayListIterator<T,N>::equals(const ConstArrayListIterator<MemberType,N>& other) const
  {
    // Makes only sense if we reference a common list
    assert(list_==(other.list_));
    return position_==other.position_ ;
  }


  template<class T, int N>
  bool ConstArrayListIterator<T,N>::equals(const ConstArrayListIterator<MemberType,N>& other) const
  {
    // Makes only sense if we reference a common list
    assert(list_==(other.list_));
    return position_==other.position_ ;
  }

  template<class T, int N>
  void ArrayListIterator<T,N>::increment()
  {
    ++position_;
  }

  template<class T, int N>
  void ConstArrayListIterator<T,N>::increment()
  {
    ++position_;
  }

  template<class T, int N>
  void ArrayListIterator<T,N>::decrement()
  {
    --position_;
  }

  template<class T, int N>
  void ConstArrayListIterator<T,N>::decrement()
  {
    --position_;
  }

  template<class T, int N>
  T& ArrayListIterator<T,N>::elementAt(int i) const
  {
    i+=position_;
    return list_->elementAt(i+position_);
  }

  template<class T, int N>
  const T& ConstArrayListIterator<T,N>::elementAt(int i) const
  {
    return list_->elementAt(i+position_);
  }

  template<class T, int N>
  T& ArrayListIterator<T,N>::dereference() const
  {
    return list_->elementAt(position_);
  }

  template<class T, int N>
  const T& ConstArrayListIterator<T,N>::dereference() const
  {
    return list_->elementAt(position_);
  }

  template<class T, int N>
  int ArrayListIterator<T,N>::distanceTo(const ArrayListIterator<T,N>& other) const
  {
    // Makes only sense if we reference a common list
    assert(list_==(other.list_));
    return other.position_ - position_;
  }

  template<class T, int N>
  int ConstArrayListIterator<T,N>::distanceTo(const ConstArrayListIterator<T,N>& other) const
  {
    // Makes only sense if we reference a common list
    assert(list_==(other.list_));
    return other.position_ - position_;
  }

  template<class T, int N>
  ArrayListIterator<T,N>& ArrayListIterator<T,N>::operator=(const ArrayListIterator<T,N>& other)
  {
    position_=other.position_;
    list_=other.list_;
    return *this;
  }

  template<class T, int N>
  const ConstArrayListIterator<T,N>& ConstArrayListIterator<T,N>::operator=(const ConstArrayListIterator<T,N>& other)
  {
    position_=other.position_;
    list_=other.list_;
    return *this;
  }

  template<class T, int N>
  void ArrayListIterator<T,N>::eraseToHere()
  {
    list_->size_ -= ++position_ - list_->start_;
    int chunks = position_/chunkSize_;
    list_->start_ = position_ % chunkSize_;
    position_ = list_->start_;
    if(chunks>0) {
      typedef typename std::vector<SmartPointer<FixedArray<MemberType,chunkSize_> > >::iterator iterator;
      iterator chunk=list_->chunks_.begin(), chunk1=chunk+chunks;
      list_->chunks_.erase(chunk,chunk1);
    }
    list_->capacity_-=chunks*chunkSize_;
  }

  template<class T, int N>
  ArrayListIterator<T,N>::ArrayListIterator(ArrayList<T,N>& arrayList, int position)
    : position_(position), list_(&arrayList)
  {}


  template<class T, int N>
  ConstArrayListIterator<T,N>::ConstArrayListIterator(const ArrayList<T,N>& arrayList, int position)
    : position_(position), list_(&arrayList)
  {}

  template<class T, int N>
  ConstArrayListIterator<T,N>::ConstArrayListIterator(const ArrayListIterator<T,N>& other)
    : position_(other.position_), list_(other.list_)
  {}


  /** @} */
}
#endif
