// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE__SLLIST_HH
#define DUNE__SLLIST_HH

#include <memory>
#include "iteratorfacades.hh"

namespace Dune
{
  /**
   * @file
   * This file implements a single linked list together with
   * the necessary iterators.
   * @author Markus Blatt
   */
  /**
   * @addtogroup Common
   *
   * @{
   */
  template<typename T, class A>
  class SLListIterator;

  template<typename T, class A>
  class SLListConstIterator;

  /**
   * @brief A single linked list.
   *
   * The list is capabale of insertions at the front and at
   * the end and of removing elements at the front. Those
   * operations require constant time.
   */
  template<typename T, class A=std::allocator<T> >
  class SLList
  {
    class Element;
    friend class SLListIterator<T,A>;
    friend class SLListConstIterator<T,A>;

  public:

    /**
     * @brief The type we store.
     */
    typedef T MemberType;

    /**
     * @brief The allocator to use.
     */
    typedef typename A::template rebind<Element>::other Allocator;

    /**
     * @brief The mutable iterator of the list.
     */
    typedef SLListIterator<T,A> iterator;

    /**
     * @brief The mutable iterator of the list.
     */
    typedef SLListConstIterator<T,A> const_iterator;
    SLList();

    /**
     * @brief Add a new entry to the end of the list.
     * @param item The item to add.
     */
    inline void push_back(const MemberType& item);

    /**
     * @brief Add a new entry to the beginning of the list.
     * @param item The item to add.
     */
    inline void push_front(const MemberType& item);

    /**
     * @brief Remove the first item in the list.
     */
    inline void pop_front();

    /** @brief Remove all elements from the list. */
    inline void clear();

    /**
     * @brief Get an iterator pointing to the first
     * element in the list.
     *
     * @return An iterator pointing to the first
     * element or the end if the list is empty.
     */
    inline iterator begin();

    /**
     * @brief Get an iterator pointing to the first
     * element in the list.
     *
     * @return An iterator pointing to the first
     * element or the end if the list is empty.
     */
    inline const_iterator begin() const;
    /**
     * @brief Get an iterator pointing to the
     * end of the list.
     *
     * @return An iterator pointing to the end.
     */
    inline iterator end();

    /**
     * @brief Get an iterator pointing to the
     * end of the list.
     *
     * @return An iterator pointing to the end.
     */
    inline const_iterator end() const;

    /**
     * @brief Check whether the list is empty.
     *
     * @return True if the list is empty;
     */
    inline bool empty() const;

    /**
     * @brief Get the number of elements the list
     * contains.
     */
    inline int size() const;

  private:
    struct Element
    {
      /**
       * @brief The next element in the list.
       */
      Element* next_;
      /**
       * @brief The element we hold.
       */
      MemberType item_;

      Element(const MemberType& item);

    };

    /** @brief The first element in the list. */
    Element* head_;

    /** @brief The last element in the list. */
    Element* tail_;

    /** @brief The allocator we use. */
    Allocator allocator_;

    /** brief The number of elements the list holds. */
    int size_;
  };

  /**
   * @brief A mutable iterator for the SLList.
   */
  template<typename T, class A>
  class SLListIterator : public Dune::ForwardIteratorFacade<SLListIterator<T,A>, T, T&, std::size_t>
  {
    friend class SLListConstIterator<T,A>;

  public:
    inline SLListIterator(typename SLList<T,A>::Element* item)
      : current_(item)
    {}

    inline SLListIterator()
      : current_(0)
    {}

    /**
     * @brief Dereferencing function for the iterator facade.
     * @return A reference to the element at the current position.
     */
    inline T& dereference() const
    {
      return current_->item_;
    }

    /**
     * @brief Equality test for the iterator facade.
     * @param other The other iterator to check.
     * @return true If the other iterator is at the same position.
     */
    inline bool equals(const SLListConstIterator<T,A>& other) const
    {
      return current_==other.current_;
    }


    /**
     * @brief Equality test for the iterator facade.
     * @param other The other iterator to check.
     * @return true If the other iterator is at the same position.
     */
    inline bool equals(const SLListIterator<T,A>& other) const
    {
      return current_==other.current_;
    }

    /**
     * @brief Increment function for the iterator facade.
     */
    inline void increment()
    {
      current_ = current_->next_;
    }

  private:
    /**  @brief The current element. */
    typename SLList<T,A>::Element* current_;
  };

  /**
   * @brief A constant iterator for the SLList.
   */
  template<class T, class A>
  class SLListConstIterator : public Dune::ForwardIteratorFacade<SLListConstIterator<T,A>, const T, const T&, std::size_t>
  {
    friend class SLListIterator<T,A>;

  public:
    inline SLListConstIterator()
      : current_(0)
    {}

    inline SLListConstIterator(typename SLList<T,A>::Element* item)
      : current_(item)
    {}

    inline SLListConstIterator(const SLListIterator<T,A>& other)
    {
      current_=other.current_;
    }

    /**
     * @brief Dereferencing function for the facade.
     * @return A reference to the element at the current position.
     */
    inline const T& dereference() const
    {
      return current_->item_;
    }

    /**
     * @brief Equality test for the iterator facade.
     * @param other The other iterator to check.
     * @return true If the other iterator is at the same position.
     */
    inline bool equals(const SLListConstIterator<T,A>& other) const
    {
      return current_==other.current_;
    }


    /**
     * @brief Equality test for the iterator facade.
     * @param other The other iterator to check.
     * @return true If the other iterator is at the same position.
     */
    inline bool equals(const SLListIterator<T,A>& other) const
    {
      return current_==other.current_;
    }

    /**
     * @brief Increment function for the iterator facade.
     */
    inline void increment()
    {
      current_ = current_->next_;
    }

  private:
    /**  @brief The current element. */
    typename SLList<T,A>::Element* current_;
  };


  template<typename T, class A>
  SLList<T,A>::Element::Element(const T& item)
    : item_(item)
  {}

  template<typename T, class A>
  SLList<T,A>::SLList()
    : head_(0), tail_(0), allocator_()
  {}

  template<typename T, class A>
  inline void SLList<T,A>::push_back(const T& item)
  {
    if(tail_!=0) {
      tail_->next_ = allocator_.allocate(1);
      tail_ = tail_->next_;
      tail_->item_=item;
      tail_->next_=0;
    }else{
      tail_=head_=allocator_.allocate(1);
      tail_->next_=0;
      tail_->item_=item;
    }
    ++size_;
  }

  template<typename T, class A>
  inline void SLList<T,A>::push_front(const T& item)
  {
    if(head_==0) {
      head_ = tail_ = allocator_.allocate(1);
      head_->item_=item;
      head_->next_=0;
    }else{
      Element* added = allocator_.allocate(1);
      added->item_=item;
      added->next_=head_;
      head_=added;
    }
    ++size_;
  }

  template<typename T, class A>
  inline void SLList<T,A>::pop_front()
  {
    assert(head_!=0);
    Element* tmp = head_;
    head_=head_->next_;
    allocator_.destroy(tmp);
    allocator_.deallocate(tmp, 1);
    --size_;
  }

  template<typename T, class A>
  inline void SLList<T,A>::clear()
  {
    while(head_) {
      Element* current = head_;
      head_=current->next_;
      allocator_.destroy(current);
      allocator_.deallocate(current, 1);
    }
    tail_ = head_;
    size_=0;
  }

  template<typename T, class A>
  inline bool SLList<T,A>::empty() const
  {
    return head_==0;
  }

  template<typename T, class A>
  inline int SLList<T,A>::size() const
  {
    return size_;
  }

  template<typename T, class A>
  inline SLListIterator<T,A> SLList<T,A>::begin()
  {
    return iterator(head_);
  }

  template<typename T, class A>
  inline SLListConstIterator<T,A> SLList<T,A>::begin() const
  {
    return const_iterator(head_);
  }


  template<typename T, class A>
  inline SLListIterator<T,A> SLList<T,A>::end()
  {
    return iterator(static_cast<Element*>(0));
  }

  template<typename T, class A>
  inline SLListConstIterator<T,A> SLList<T,A>::end() const
  {
    return const_iterator(static_cast<Element*>(0));
  }
}
/** @} */
#endif
