// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMMON_RESERVEDVECTOR_HH
#define DUNE_COMMON_RESERVEDVECTOR_HH

/** \file
 * \brief An stl-compliant random-access container which stores everything on the stack
 */

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <cstddef>
#include <dune/common/genericiterator.hh>
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
  public:

    /** @{ Typedefs */

    //! The type of object, T, stored in the vector.
    typedef T value_type;
    //! Pointer to T.
    typedef T* pointer;
    //! Const pointer to T.
    typedef T const* const_pointer;
    //! Reference to T
    typedef T& reference;
    //! Const reference to T
    typedef const T& const_reference;
    //! An unsigned integral type.
    typedef size_t size_type;
    //! A signed integral type.
    typedef std::ptrdiff_t difference_type;
    //! Iterator used to iterate through a vector.
    typedef Dune::GenericIterator<ReservedVector, value_type> iterator;
    //! Const iterator used to iterate through a vector.
    typedef Dune::GenericIterator<const ReservedVector, const value_type> const_iterator;
    //! Reverse iterator
    typedef std::reverse_iterator<iterator> reverse_iterator;
    //! Const reverse iterator
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    /** @} */

    /** @{ Constructors */

    //! Constructor
    constexpr ReservedVector() noexcept = default;

    constexpr ReservedVector(std::initializer_list<T> const &l) noexcept
    {
      assert(l.size() <= n);// Actually, this is not needed any more! Why?
      size_ = l.size();
      auto it = l.begin();
      for (size_type i=0; i<size_ && it!=l.end(); ++i)
        storage_[i] = *it++;
    }

    /** @} */

    constexpr bool operator== (const ReservedVector & other) const noexcept
    {
      bool eq = (size_ == other.size_);
      for (size_type i=0; i<size_ && eq; ++i)
        eq = eq && (storage_[i] == other.storage_[i]);
      return eq;
    }

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
    constexpr void push_back(const T& t) noexcept
    {
      CHECKSIZE(size_<n);
      storage_[size_++] = t;
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
      return iterator(*this, 0);
    }

    //! Returns a const_iterator pointing to the beginning of the vector.
    constexpr const_iterator begin() const noexcept
    {
      return const_iterator(*this, 0);
    }

    //! Returns a const_iterator pointing to the beginning of the vector.
    constexpr const_iterator cbegin() const noexcept
    {
      return const_iterator(*this, 0);
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
      return iterator(*this, size_);
    }

    //! Returns a const_iterator pointing to the end of the vector.
    constexpr const_iterator end() const noexcept
    {
      return const_iterator(*this, size_);
    }

    //! Returns a const_iterator pointing to the end of the vector.
    constexpr const_iterator cend() const noexcept
    {
      return const_iterator(*this, size_);
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
        DUNE_THROW(Dune::RangeError, "Index out of range");
      return storage_[i];
    }

    //! Returns a const reference to the i'th element.
    constexpr const_reference at(size_type i) const
    {
      if (!(i < size()))
        DUNE_THROW(Dune::RangeError, "Index out of range");
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
    constexpr void fill(const value_type& value) noexcept
    {
      for (size_type i=0; i<size(); ++i)
        storage_[i] = value;
    }

    //! Swap the content with another vector
    void swap(ReservedVector& other) noexcept(std::is_nothrow_swappable_v<T>)
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
    std::array<T,n> storage_ = {};
    size_type size_ = 0;
  };

}

DUNE_DEFINE_HASH(DUNE_HASH_TEMPLATE_ARGS(typename T, int n),DUNE_HASH_TYPE(Dune::ReservedVector<T,n>))

#undef CHECKSIZE

#endif // DUNE_COMMON_RESERVEDVECTOR_HH
