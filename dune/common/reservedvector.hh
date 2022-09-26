// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_COMMON_RESERVEDVECTOR_HH
#define DUNE_COMMON_RESERVEDVECTOR_HH

/** \file
 * \brief An stl-compliant random-access container which stores everything on the stack
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <iterator>
#include <cstddef>
#include <initializer_list>

#include <dune/common/hash.hh>

#ifdef CHECK_RESERVEDVECTOR
#define CHECKSIZE(X) assert(X)
#else
#define CHECKSIZE(X) {}
#endif

namespace Dune
{
  /**
     \brief A Vector class with statically reserved memory.

     ReservedVector is something between std::array and std::vector.
     It is a dynamically sized vector which can be extended and shrunk
     using methods like push_back and pop_back, but reserved memory is
     statically predefined.

     This implies that the vector cannot grow bigger than the predefined
     maximum size.

     \tparam T The value type ReservedVector stores.
     \tparam n The maximum number of objects the ReservedVector can store.

   */
  template<class T, int n>
  class ReservedVector
  {
    using storage_type = std::array<T,n>;

  public:

    /** @{ Typedefs */

    //! The type of object, T, stored in the vector.
    typedef typename storage_type::value_type value_type;
    //! Pointer to T.
    typedef typename storage_type::pointer pointer;
    //! Const pointer to T.
    typedef typename storage_type::const_pointer const_pointer;
    //! Reference to T
    typedef typename storage_type::reference reference;
    //! Const reference to T
    typedef typename storage_type::const_reference const_reference;
    //! An unsigned integral type.
    typedef typename storage_type::size_type size_type;
    //! A signed integral type.
    typedef typename storage_type::difference_type difference_type;
    //! Iterator used to iterate through a vector.
    typedef typename storage_type::iterator iterator;
    //! Const iterator used to iterate through a vector.
    typedef typename storage_type::const_iterator const_iterator;
    //! Reverse iterator
    typedef std::reverse_iterator<iterator> reverse_iterator;
    //! Const reverse iterator
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    /** @} */

    /** @{ Constructors */

    //! Constructs an empty vector
    constexpr ReservedVector()
          noexcept(std::is_nothrow_default_constructible_v<value_type>)
      : storage_()
      , size_(0)
    {}

    //! Constructs the vector with `count` elements that will be default-initialized.
    explicit constexpr ReservedVector(size_type count)
          noexcept(std::is_nothrow_default_constructible_v<value_type>)
      : storage_()
      , size_(count)
    {
      assert(count <= n);
    }

    //! Constructs the vector with `count` copies of elements with value `value`.
    constexpr ReservedVector(size_type count, const value_type& value)
          noexcept(std::is_nothrow_copy_assignable_v<value_type> &&
          noexcept(ReservedVector(count)))
      : ReservedVector(count)
    {
      for (size_type i=0; i<count; ++i)
        storage_[i] = value;
    }

    //! Constructs the vector from an iterator range `[first,last)`
    template<class InputIt,
      std::enable_if_t<std::is_convertible_v<typename std::iterator_traits<InputIt>::value_type, value_type>, int> = 0>
    constexpr ReservedVector(InputIt first, InputIt last)
          noexcept(std::is_nothrow_copy_assignable_v<value_type> &&
          noexcept(ReservedVector()))
      : ReservedVector()
    {
      for (size_type i=0; i<n && first!=last; ++i,++size_)
        storage_[i] = *first++;
      assert(first == last);
    }

    //! Constructs the vector from an initializer list
    constexpr ReservedVector(std::initializer_list<value_type> const& l)
          noexcept(std::is_nothrow_copy_assignable_v<value_type> &&
          noexcept(ReservedVector(l.begin(),l.end())))
      : ReservedVector(l.begin(),l.end())
    {}

    /** @} */

    /** @{ Comparison */

    //! Compares the values in the vector `this` with `that` for equality
    constexpr bool operator== (const ReservedVector& that) const noexcept
    {
      if (size() != that.size())
        return false;
      for (size_type i=0; i<size(); ++i)
        if (!(storage_[i]==that.storage_[i]))
          return false;
      return true;
    }

    //! Compares the values in the vector `this` with `that` for not equality
    constexpr bool operator!= (const ReservedVector& that) const noexcept
    {
      return !(*this == that);
    }

    //! Lexicographically compares the values in the vector `this` with `that`
    constexpr bool operator< (const ReservedVector& that) const noexcept
    {
      for (size_type i=0; i<std::min(size(),that.size()); ++i) {
        if (storage_[i] < that.storage_[i]) return true;
        if (that.storage_[i] < storage_[i]) return false;
      }
      return size() < that.size();
    }

    //! Lexicographically compares the values in the vector `this` with `that`
    constexpr bool operator> (const ReservedVector& that) const noexcept
    {
      return that < *this;
    }

    //! Lexicographically compares the values in the vector `this` with `that`
    constexpr bool operator<= (const ReservedVector& that) const noexcept
    {
      return !(*this > that);
    }

    //! Lexicographically compares the values in the vector `this` with `that`
    constexpr bool operator>= (const ReservedVector& that) const noexcept
    {
      return !(*this < that);
    }

    /** @} */

    /** @{ Modifiers */

    //! Erases all elements.
    constexpr void clear() noexcept
    {
      size_ = 0;
    }

    //! Specifies a new size for the vector.
    constexpr void resize(size_type s) noexcept
    {
      CHECKSIZE(s<=n);
      size_ = s;
    }

    //! Appends an element to the end of a vector, up to the maximum size n, O(1) time.
    constexpr void push_back(const value_type& t)
          noexcept(std::is_nothrow_copy_assignable_v<value_type>)
    {
      CHECKSIZE(size_<n);
      storage_[size_++] = t;
    }

    //! Appends an element to the end of a vector by moving the value, up to the maximum size n, O(1) time.
    constexpr void push_back(value_type&& t)
          noexcept(std::is_nothrow_move_assignable_v<value_type>)
    {
      CHECKSIZE(size_<n);
      storage_[size_++] = std::move(t);
    }

    //! Appends an element to the end of a vector by constructing it in place
    template<class... Args>
    reference emplace_back(Args&&... args)
          noexcept(std::is_nothrow_constructible_v<value_type,decltype(args)...>)
    {
      CHECKSIZE(size_<n);
      value_type* p = &storage_[size_++];
      // first destroy any previously (default) constructed element at that location
      p->~value_type();
      // construct the value_type in place
      // NOTE: This is not an integral constant expression.
      // With c++20 we could use std::construct_at
      ::new (const_cast<void*>(static_cast<const volatile void*>(p)))
          value_type(std::forward<Args>(args)...);
      return *p;
    }

    //! Erases the last element of the vector, O(1) time.
    constexpr void pop_back() noexcept
    {
      if (! empty()) size_--;
    }

    /** @} */

    /** @{ Iterators  */

    //! Returns a iterator pointing to the beginning of the vector.
    constexpr iterator begin() noexcept
    {
      return storage_.begin();
    }

    //! Returns a const_iterator pointing to the beginning of the vector.
    constexpr const_iterator begin() const noexcept
    {
      return storage_.begin();
    }

    //! Returns a const_iterator pointing to the beginning of the vector.
    constexpr const_iterator cbegin() const noexcept
    {
      return storage_.cbegin();
    }

    //! Returns a const reverse-iterator pointing to the end of the vector.
    constexpr reverse_iterator rbegin() noexcept
    {
      return reverse_iterator{begin()+size()};
    }

    //! Returns a const reverse-iterator pointing to the end of the vector.
    constexpr const_reverse_iterator rbegin() const noexcept
    {
      return const_reverse_iterator{begin()+size()};
    }

    //! Returns a const reverse-iterator pointing to the end of the vector.
    constexpr const_reverse_iterator crbegin() const noexcept
    {
      return const_reverse_iterator{begin()+size()};
    }

    //! Returns an iterator pointing to the end of the vector.
    constexpr iterator end() noexcept
    {
      return storage_.begin()+size();
    }

    //! Returns a const_iterator pointing to the end of the vector.
    constexpr const_iterator end() const noexcept
    {
      return storage_.begin()+size();
    }

    //! Returns a const_iterator pointing to the end of the vector.
    constexpr const_iterator cend() const noexcept
    {
      return storage_.cbegin()+size();
    }

    //! Returns a const reverse-iterator pointing to the begin of the vector.
    constexpr reverse_iterator rend() noexcept
    {
      return reverse_iterator{begin()};
    }

    //! Returns a const reverse-iterator pointing to the begin of the vector.
    constexpr const_reverse_iterator rend() const noexcept
    {
      return const_reverse_iterator{begin()};
    }

    //! Returns a const reverse-iterator pointing to the begin of the vector.
    constexpr const_reverse_iterator crend() const noexcept
    {
      return const_reverse_iterator{begin()};
    }

    /** @} */

    /** @{ Element access  */

    //! Returns reference to the i'th element.
    constexpr reference at(size_type i)
    {
      if (!(i < size()))
        throw std::out_of_range("Index out of range");
      return storage_[i];
    }

    //! Returns a const reference to the i'th element.
    constexpr const_reference at(size_type i) const
    {
      if (!(i < size()))
        throw std::out_of_range("Index out of range");
      return storage_[i];
    }

    //! Returns reference to the i'th element.
    constexpr reference operator[] (size_type i) noexcept
    {
      CHECKSIZE(size_>i);
      return storage_[i];
    }

    //! Returns a const reference to the i'th element.
    constexpr const_reference operator[] (size_type i) const noexcept
    {
      CHECKSIZE(size_>i);
      return storage_[i];
    }

    //! Returns reference to first element of vector.
    constexpr reference front() noexcept
    {
      CHECKSIZE(size_>0);
      return storage_[0];
    }

    //! Returns const reference to first element of vector.
    constexpr const_reference front() const noexcept
    {
      CHECKSIZE(size_>0);
      return storage_[0];
    }

    //! Returns reference to last element of vector.
    constexpr reference back() noexcept
    {
      CHECKSIZE(size_>0);
      return storage_[size_-1];
    }

    //! Returns const reference to last element of vector.
    constexpr const_reference back() const noexcept
    {
      CHECKSIZE(size_>0);
      return storage_[size_-1];
    }

    //! Returns pointer to the underlying memory.
    constexpr pointer data() noexcept
    {
      return storage_.data();
    }

    //! Returns const pointer to the underlying memory.
    constexpr const_pointer data() const noexcept
    {
      return storage_.data();
    }

    /** @} */

    /** @{ Capacity */

    //! Returns number of elements in the vector.
    constexpr size_type size() const noexcept
    {
      return size_;
    }

    //! Returns true if vector has no elements.
    constexpr bool empty() const noexcept
    {
      return size_==0;
    }

    //! Returns current capacity (allocated memory) of the vector.
    static constexpr size_type capacity() noexcept
    {
      return n;
    }

    //! Returns the maximum length of the vector.
    static constexpr size_type max_size() noexcept
    {
      return n;
    }

    /** @} */

    /** @{ Operations */

    //! Fill the container with the value
    constexpr void fill(const value_type& value)
          noexcept(std::is_nothrow_copy_assignable_v<value_type>)
    {
      for (size_type i=0; i<size(); ++i)
        storage_[i] = value;
    }

    //! Swap the content with another vector
    void swap(ReservedVector& other)
          noexcept(std::is_nothrow_swappable_v<value_type>)
    {
      using std::swap;
      swap(storage_, other.storage_);
      swap(size_, other.size_);
    }

    /** @} */

    //! Send ReservedVector to an output stream
    friend std::ostream& operator<< (std::ostream& s, const ReservedVector& v)
    {
      for (size_type i=0; i<v.size(); i++)
        s << v[i] << "  ";
      return s;
    }

    inline friend std::size_t hash_value(const ReservedVector& v) noexcept
    {
      return hash_range(v.storage_.data(),v.storage_.data()+v.size_);
    }

  private:
    storage_type storage_;
    size_type size_;
  };

}

DUNE_DEFINE_HASH(DUNE_HASH_TEMPLATE_ARGS(typename T, int n),DUNE_HASH_TYPE(Dune::ReservedVector<T,n>))

#undef CHECKSIZE

#endif // DUNE_COMMON_RESERVEDVECTOR_HH
