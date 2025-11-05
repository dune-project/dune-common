// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_INDEXEDITERATOR_HH
#define DUNE_COMMON_INDEXEDITERATOR_HH

#include <iterator>
#include <type_traits>

#include <dune/common/iteratorfacades.hh>

namespace Dune
{
  /**
   * \brief An iterator mixin that adds an `index()` method returning an
   * enumeration index for the traversal.
   *
   * This is a mixin class that derives from its template parameter `Iter` and
   * adds the method `index()` to retrieve an enumeration index during traversal.
   * Only the increment and decrement operators are adapted to also increment
   * (decrement) the accompanying index in the same way the iterator is incremented
   * or decremented. Especially the dereference and comparison operators are not
   * modified by this mixin class.
   *
   * \tparam Iter  An iterator type from which the `IndexedIterator` will be derived.
   */
  template <class Iter>
  class IndexedIterator
      : public Iter
  {
    using Traits = std::iterator_traits<Iter>;
    static_assert(std::is_copy_constructible_v<Iter>);
    static_assert(std::is_base_of_v<std::forward_iterator_tag, typename Traits::iterator_category>);

  public:
    //! Type used for storing the traversal index
    using size_type = typename Traits::difference_type;

    //! Default constructor default-constructs the `Iter` base type and the index by 0.
    template <class I = Iter,
      std::enable_if_t<std::is_default_constructible_v<I>, bool> = true>
    constexpr IndexedIterator ()
          noexcept(std::is_nothrow_default_constructible_v<Iter>)
    {}

    //! Construct the `Iter` base type by `it` and the index by the given starting index.
    //! Note that this constructor allows implicit conversion from `Iter` type.
    constexpr IndexedIterator (Iter it, size_type index = 0)
          noexcept(std::is_nothrow_copy_constructible_v<Iter>)
      : Iter(it)
      , index_(index)
    {}

    //! Return the enumeration index.
    [[nodiscard]] constexpr size_type index () const noexcept
    {
      return index_;
    }

    //! Increment the iterator and the index.
    constexpr IndexedIterator& operator++ ()
    {
      Iter::operator++();
      ++index_;
      return *this;
    }

    //! Increment the iterator and the index.
    constexpr IndexedIterator operator++ (int)
    {
      IndexedIterator tmp(*this);
      this->operator++();
      return tmp;
    }

    //! Decrement the iterator and the index.
    template <class I = Iter,
      decltype(--std::declval<I&>(), bool{}) = true>
    constexpr IndexedIterator& operator-- ()
    {
      Iter::operator--();
      --index_;
      return *this;
    }

    //! Decrement the iterator and the index.
    template <class I = Iter,
      decltype(std::declval<I&>()--, bool{}) = true>
    constexpr IndexedIterator operator-- (int)
    {
      IndexedIterator tmp(*this);
      this->operator--();
      return tmp;
    }

    //! Increment the iterator and the index.
    template <class I = Iter,
      decltype(std::declval<I&>()+=1, bool{}) = true>
    constexpr IndexedIterator& operator+= (typename Iter::difference_type n)
    {
      Iter::operator+=(n);
      index_ += n;
      return *this;
    }

    //! Decrement the iterator and the index.
    template <class I = Iter,
      decltype(std::declval<I&>()-=1, bool{}) = true>
    constexpr IndexedIterator& operator-= (typename Iter::difference_type n)
    {
      Iter::operator-=(n);
      index_ -= n;
      return *this;
    }

  private:
    size_type index_ = 0;
  };


  /// \brief Specialization for pointer types
  template <class T>
  class IndexedIterator<T*>
    : public Dune::IteratorFacade<IndexedIterator<T*>, std::random_access_iterator_tag, T>
  {
    using Facade = Dune::IteratorFacade<IndexedIterator<T*>, std::random_access_iterator_tag, T>;

  public:
    using reference = typename Facade::reference;
    using difference_type = typename Facade::difference_type;
    using iterator_type = T*;

    //! Construct an IndexedIterator from a pointer and an index.
    constexpr explicit IndexedIterator (T* ptr, difference_type index = 0)
      : ptr_(ptr)
      , index_(index)
    {}

    //! Dereference the pointer.
    constexpr reference operator* () const { return *ptr_; }

    //! Increment the iterator and the index.
    constexpr IndexedIterator& operator+= (typename Facade::difference_type d)
    {
      ptr_ += d;
      index_ += d;
      return *this;
    }

    //! Decrement the iterator and the index.
    constexpr IndexedIterator& operator-= (typename Facade::difference_type d)
    {
      ptr_ -= d;
      index_ -= d;
      return *this;
    }

    //! Decrement the iterator and the index.
    friend constexpr difference_type operator- (const IndexedIterator& it1, const IndexedIterator& it2)
    {
      return it1.ptr_ - it2.ptr_;
    }

    //! Compare two iterators for equality: compares the underlying ptrs.
    friend constexpr bool operator== (const IndexedIterator& it1, const IndexedIterator& it2)
    {
      return it1.ptr_ == it2.ptr_;
    }

    //! Return the enumeration index.
    [[nodiscard]] constexpr difference_type index () const noexcept
    {
      return index_;
    }

  private:
    T* ptr_ = nullptr;
    difference_type index_ = 0;
  };

} // end namespace Dune

#endif // DUNE_COMMON_INDEXEDITERATOR_HH
