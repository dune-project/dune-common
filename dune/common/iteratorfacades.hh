// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_ITERATORFACADES_HH
#define DUNE_ITERATORFACADES_HH

#include <iterator>
#include <type_traits>

#include <dune/common/typetraits.hh>
#include <dune/common/concept.hh>

namespace Dune
{
  /*! \defgroup IteratorFacades Iterator facades
     \ingroup Common

     \brief Iterator facades for writing stl conformant iterators.

     With using these facades writing iterators for arbitrary containers becomes much less
     cumbersome as only few functions have to be implemented. All other functions needed by
     the stl are provided by the facades using the Barton-Nackman trick (also known as
     curiously recurring template pattern).

     The following example illustrates how a random access iterator might be written:

     \code
     #include<dune/common/iteratorfacades.hh>

     ...

     template<class C, class T>
     class TestIterator : public Dune::BidirectionalIteratorFacade<TestIterator<C,T>,T, T&, int>
     {
      friend class TestIterator<typename std::remove_const<C>::type, typename std::remove_const<T>::type >;
      friend class TestIterator<const typename std::remove_const<C>::type, const typename std::remove_const<T>::type >;

     public:

      // Constructors needed by the facade iterators.
      TestIterator(): container_(0), position_(0)
      { }

      TestIterator(C& cont, int pos)
        : container_(&cont), position_(pos)
      {}

      TestIterator(const TestIterator<typename std::remove_const<C>::type, typename std::remove_const<T>::type >& other)
        : container_(other.container_), position_(other.position_)
      {}


      TestIterator(const TestIterator<const typename std::remove_const<C>::type, const typename std::remove_const<T>::type >& other)
        : container_(other.container_), position_(other.position_)
      {}

      // Methods needed by the forward iterator
      bool equals(const TestIterator<typename std::remove_const<C>::type,typename std::remove_const<T>::type>& other) const
      {
        return position_ == other.position_ && container_ == other.container_;
      }


      bool equals(const TestIterator<const typename std::remove_const<C>::type,const typename std::remove_const<T>::type>& other) const
      {
        return position_ == other.position_ && container_ == other.container_;
      }

      T& dereference() const
      {
        return container_->values_[position_];
      }

      void increment()
      {
     ++position_;
      }

      // Additional function needed by BidirectionalIterator
      void decrement()
     {
        --position_;
      }

      // Additional function needed by RandomAccessIterator
      T& elementAt(int i)const
      {
        return container_->operator[](position_+i);
      }

      void advance(int n)
      {
        position_=position_+n;
      }

      std::ptrdiff_t distanceTo(TestIterator<const typename std::remove_const<C>::type,const typename std::remove_const<T>::type> other) const
      {
        assert(other.container_==container_);
        return other.position_ - position_;
      }

      std::ptrdiff_t distanceTo(TestIterator<const typename std::remove_const<C>::type, typename std::remove_const<T>::type> other) const
      {
        assert(other.container_==container_);
        return other.position_ - position_;
      }
     private:
      C *container_;
      size_t position_;
     };

     \endcode
     See dune/common/test/iteratorbase.hh for details.
   */


  /**
   * @file
   * @brief This file implements iterator facade classes for writing stl conformant iterators.
   *
   * With using these facades writing iterators for arbitrary containers becomes much less
   * cumbersome as only few functions have to be implemented. All other functions needed by
   * the stl are provided by the facades using the Barton-Nackman trick (also known as
   * curiously recurring template pattern.
   */

  /** @addtogroup IteratorFacades
   *
   * @{
   */
  /**
   * @brief Base class for stl conformant forward iterators.
   *
   * \tparam T The derived class
   * \tparam V The value type
   * \tparam R The reference type
   * \tparam D The type for differences between two iterators
   */
  template<class T, class V, class R = V&, class D = std::ptrdiff_t>
  class ForwardIteratorFacade
  {

  public:
    /* type aliases required by C++ for iterators */
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename std::remove_const<V>::type;
    using difference_type = D;
    using pointer = V*;
    using reference = R;

    /**
     * @brief The type of derived iterator.
     *
     * The iterator has to define following
     * functions have to be present:
     *
     * \code
     *
     * // Access the value referred to.
     * Reference dereference() const;
     *
     * // Compare for equality with iterator j
     * bool equals(j);
     *
     * // position the iterator at the next element.
     * void increment()
     *
     * // check for equality with other iterator
     * bool equals(other)
     * \endcode
     *
     * For an elaborate explanation see the
     * <A HREF="http://www.sgi.com/tech/stl/iterator_traits.html">STL Documentation</A>!
     */
    typedef T DerivedType;

    /**
     * @brief The type of value accessed through the iterator.
     */
    typedef V Value;

    /**
     * @brief The pointer to the Value.
     */
    typedef V* Pointer;

    /**
     * @brief The type of the difference between two positions.
     */
    typedef D DifferenceType;

    /**
     * @brief The type of the reference to the values accessed.
     */
    typedef R Reference;

    /** @brief Dereferencing operator. */
    Reference operator*() const
    {
      return static_cast<DerivedType const*>(this)->dereference();
    }

    Pointer operator->() const
    {
      return &(static_cast<const DerivedType *>(this)->dereference());
    }

    /** @brief Preincrement operator. */
    DerivedType& operator++()
    {
      static_cast<DerivedType *>(this)->increment();
      return *static_cast<DerivedType *>(this);
    }

    /** @brief Postincrement operator. */
    DerivedType operator++(int)
    {
      DerivedType tmp(static_cast<DerivedType const&>(*this));
      this->operator++();
      return tmp;
    }
  };

  /**
   * @brief Checks for equality.
   *
   * This operation is only defined if either D2
   * is convertible to D1 or vice versa. If that is
   * not the case the compiler will report an error
   * as EnableIfInterOperable<D1,D2,bool>::type is
   * not defined.
   *
   */
  template<class T1, class V1, class R1, class D,
      class T2, class V2, class R2>
  inline typename EnableIfInterOperable<T1,T2,bool>::type
  operator==(const ForwardIteratorFacade<T1,V1,R1,D>& lhs,
             const ForwardIteratorFacade<T2,V2,R2,D>& rhs)
  {
    if(std::is_convertible<T2,T1>::value)
      return static_cast<const T1&>(lhs).equals(static_cast<const T2&>(rhs));
    else
      return static_cast<const T2&>(rhs).equals(static_cast<const T1&>(lhs));
  }

  /**
   * @brief Checks for inequality.
   *
   * This operation is only defined if either D2
   * is convertible to D1 or vice versa. If that is
   * not the case the compiler will report an error
   * as EnableIfInterOperable<D1,D2,bool>::type is
   * not defined.
   *
   */
  template<class T1, class V1, class R1, class D,
      class T2, class V2, class R2>
  inline typename EnableIfInterOperable<T1,T2,bool>::type
  operator!=(const ForwardIteratorFacade<T1,V1,R1,D>& lhs,
             const ForwardIteratorFacade<T2,V2,R2,D>& rhs)
  {
    if(std::is_convertible<T2,T1>::value)
      return !static_cast<const T1&>(lhs).equals(static_cast<const T2&>(rhs));
    else
      return !static_cast<const T2&>(rhs).equals(static_cast<const T1&>(lhs));
  }

  /**
   * @brief Facade class for stl conformant bidirectional iterators.
   *
   */
  template<class T, class V, class R = V&, class D = std::ptrdiff_t>
  class BidirectionalIteratorFacade
  {

  public:
    /* type aliases required by C++ for iterators */
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = typename std::remove_const<V>::type;
    using difference_type = D;
    using pointer = V*;
    using reference = R;

    /**
     * @brief The type of derived iterator.
     *
     * The iterator has to define following
     * functions have to be present:
     *
     * \code
     *
     * // Access the value referred to.
     * Reference dereference() const;
     *
     * // Compare for equality with j
     * bool equals(j);
     *
     * // position the iterator at the next element.
     * void increment()
     *
     * // position the iterator at the previous element.
     * void decrement()
     *
     * \endcode
     *
     * For an elaborate explanation see the
     * <A HREF="http://www.sgi.com/tech/stl/iterator_traits.html">STL Documentation</A>
     */
    typedef T DerivedType;

    /**
     * @brief The type of value accessed through the iterator.
     */
    typedef V Value;

    /**
     * @brief The pointer to the Value.
     */
    typedef V* Pointer;

    /**
     * @brief The type of the difference between two positions.
     */
    typedef D DifferenceType;

    /**
     * @brief The type of the reference to the values accessed.
     */
    typedef R Reference;

    /** @brief Dereferencing operator. */
    Reference operator*() const
    {
      return static_cast<DerivedType const*>(this)->dereference();
    }

    Pointer operator->() const
    {
      return &(static_cast<const DerivedType *>(this)->dereference());
    }

    /** @brief Preincrement operator. */
    DerivedType& operator++()
    {
      static_cast<DerivedType *>(this)->increment();
      return *static_cast<DerivedType *>(this);
    }

    /** @brief Postincrement operator. */
    DerivedType operator++(int)
    {
      DerivedType tmp(static_cast<DerivedType const&>(*this));
      this->operator++();
      return tmp;
    }


    /** @brief Preincrement operator. */
    DerivedType& operator--()
    {
      static_cast<DerivedType *>(this)->decrement();
      return *static_cast<DerivedType *>(this);
    }

    /** @brief Postincrement operator. */
    DerivedType operator--(int)
    {
      DerivedType tmp(static_cast<DerivedType const&>(*this));
      this->operator--();
      return tmp;
    }
  };

  /**
   * @brief Checks for equality.
   *
   * This operation is only defined if T2 is convertible to T1, otherwise it
   * is removed from the overload set since the enable_if for the return type
   * yield an invalid type expression.
   */
  template<class T1, class V1, class R1, class D,
      class T2, class V2, class R2>
  inline typename std::enable_if<std::is_convertible<T2,T1>::value,bool>::type
  operator==(const BidirectionalIteratorFacade<T1,V1,R1,D>& lhs,
             const BidirectionalIteratorFacade<T2,V2,R2,D>& rhs)
  {
    return static_cast<const T1&>(lhs).equals(static_cast<const T2&>(rhs));
  }

  /**
   * @brief Checks for equality.
   *
   * This operation is only defined if either T1 is convertible to T2, and T2
   * is not convetible to T1.  Otherwise the operator is removed from the
   * overload set since the enable_if for the return type yield an invalid
   * type expression.
   */
  template<class T1, class V1, class R1, class D,
      class T2, class V2, class R2>
  inline
  typename std::enable_if<std::is_convertible<T1,T2>::value && !std::is_convertible<T2,T1>::value,
      bool>::type
  operator==(const BidirectionalIteratorFacade<T1,V1,R1,D>& lhs,
             const BidirectionalIteratorFacade<T2,V2,R2,D>& rhs)
  {
    return static_cast<const T2&>(rhs).equals(static_cast<const T1&>(lhs));
  }

  /**
   * @brief Checks for inequality.
   *
   * This operation is only defined if either D2
   * is convertible to D1 or vice versa. If that is
   * not the case the compiler will report an error
   * as EnableIfInterOperable<D1,D2,bool>::type is
   * not defined.
   *
   */
  template<class T1, class V1, class R1, class D,
      class T2, class V2, class R2>
  inline typename EnableIfInterOperable<T1,T2,bool>::type
  operator!=(const BidirectionalIteratorFacade<T1,V1,R1,D>& lhs,
             const BidirectionalIteratorFacade<T2,V2,R2,D>& rhs)
  {
    return !(lhs == rhs);
  }

  /**
   * @brief Base class for stl conformant forward iterators.
   *
   */
  template<class T, class V, class R = V&, class D = std::ptrdiff_t>
  class RandomAccessIteratorFacade
  {

  public:
    /* type aliases required by C++ for iterators */
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename std::remove_const<V>::type;
    using difference_type = D;
    using pointer = V*;
    using reference = R;

    /**
     * @brief The type of derived iterator.
     *
     * The iterator has to define following
     * functions have to be present:
     *
     * \code
     *
     * // Access the value referred to.
     * Reference dereference() const;
     * // Access the value at some other location
     * Reference elementAt(n) const;
     *
     * // Compare for equality with j
     * bool equals(j);
     *
     * // position the iterator at the next element.
     * void increment()
     *
     * // position the iterator at the previous element.
     * void decrement()
     *
     * // advance the iterator by a number of positions-
     * void advance(DifferenceType n);
     * // calculate the distance to another iterator.
     * // One should incorporate an assertion whether
     * // the same containers are referenced
     * DifferenceType distanceTo(j) const;
     * \endcode
     *
     * For an elaborate explanation see the
     * <A HREF="http://www.sgi.com/tech/stl/iterator_traits.html">STL Documentation</A>
     */
    typedef T DerivedType;

    /**
     * @brief The type of value accessed through the iterator.
     */
    typedef V Value;

    /**
     * @brief The pointer to the Value.
     */
    typedef V* Pointer;

    /**
     * @brief The type of the difference between two positions.
     */
    typedef D DifferenceType;

    /**
     * @brief The type of the reference to the values accessed.
     */
    typedef R Reference;

    /** @brief Dereferencing operator. */
    Reference operator*() const
    {
      return static_cast<DerivedType const*>(this)->dereference();
    }

    Pointer operator->() const
    {
      return &(static_cast<const DerivedType *>(this)->dereference());
    }

    /**
     * @brief Get the element n positions from the current one.
     * @param n The distance to the element.
     * @return The element at that distance.
     */
    Reference operator[](DifferenceType n) const
    {
      return static_cast<const DerivedType *>(this)->elementAt(n);
    }

    /** @brief Preincrement operator. */
    DerivedType& operator++()
    {
      static_cast<DerivedType *>(this)->increment();
      return *static_cast<DerivedType *>(this);
    }

    /** @brief Postincrement operator. */
    DerivedType operator++(int)
    {
      DerivedType tmp(static_cast<DerivedType const&>(*this));
      this->operator++();
      return tmp;
    }

    DerivedType& operator+=(DifferenceType n)
    {
      static_cast<DerivedType *>(this)->advance(n);
      return *static_cast<DerivedType *>(this);
    }

    DerivedType operator+(DifferenceType n) const
    {
      DerivedType tmp(static_cast<DerivedType const&>(*this));
      tmp.advance(n);
      return tmp;
    }


    /** @brief Predecrement operator. */
    DerivedType& operator--()
    {
      static_cast<DerivedType *>(this)->decrement();
      return *static_cast<DerivedType *>(this);
    }

    /** @brief Postdecrement operator. */
    DerivedType operator--(int)
    {
      DerivedType tmp(static_cast<DerivedType const&>(*this));
      this->operator--();
      return tmp;
    }

    DerivedType& operator-=(DifferenceType n)
    {
      static_cast<DerivedType *>(this)->advance(-n);
      return *static_cast<DerivedType *>(this);
    }

    DerivedType operator-(DifferenceType n) const
    {
      DerivedType tmp(static_cast<DerivedType const&>(*this));
      tmp.advance(-n);
      return tmp;
    }


  };

  /**
   * @brief Checks for equality.
   *
   * This operation is only defined if either D2
   * is convertible to D1 or vice versa. If that is
   * not the case the compiler will report an error
   * as EnableIfInterOperable<D1,D2,bool>::type is
   * not defined.
   *
   */
  template<class T1, class V1, class R1, class D,
      class T2, class V2, class R2>
  inline typename EnableIfInterOperable<T1,T2,bool>::type
  operator==(const RandomAccessIteratorFacade<T1,V1,R1,D>& lhs,
             const RandomAccessIteratorFacade<T2,V2,R2,D>& rhs)
  {
    if(std::is_convertible<T2,T1>::value)
      return static_cast<const T1&>(lhs).equals(static_cast<const T2&>(rhs));
    else
      return static_cast<const T2&>(rhs).equals(static_cast<const T1&>(lhs));
  }

  /**
   * @brief Checks for inequality.
   *
   * This operation is only defined if either D2
   * is convertible to D1 or vice versa. If that is
   * not the case the compiler will report an error
   * as EnableIfInterOperable<D1,D2,bool>::type is
   * not defined.
   *
   */
  template<class T1, class V1, class R1, class D,
      class T2, class V2, class R2>
  inline typename EnableIfInterOperable<T1,T2,bool>::type
  operator!=(const RandomAccessIteratorFacade<T1,V1,R1,D>& lhs,
             const RandomAccessIteratorFacade<T2,V2,R2,D>& rhs)
  {
    if(std::is_convertible<T2,T1>::value)
      return !static_cast<const T1&>(lhs).equals(static_cast<const T2&>(rhs));
    else
      return !static_cast<const T2&>(rhs).equals(static_cast<const T1&>(lhs));
  }

  /**
   * @brief Comparison operator.
   *
   * This operation is only defined if either D2
   * is convertible to D1 or vice versa. If that is
   * not the case the compiler will report an error
   * as EnableIfInterOperable<D1,D2,bool>::type is
   * not defined.
   *
   */
  template<class T1, class V1, class R1, class D,
      class T2, class V2, class R2>
  inline typename EnableIfInterOperable<T1,T2,bool>::type
  operator<(const RandomAccessIteratorFacade<T1,V1,R1,D>& lhs,
            const RandomAccessIteratorFacade<T2,V2,R2,D>& rhs)
  {
    if(std::is_convertible<T2,T1>::value)
      return static_cast<const T1&>(lhs).distanceTo(static_cast<const T2&>(rhs))>0;
    else
      return static_cast<const T2&>(rhs).distanceTo(static_cast<const T1&>(lhs))<0;
  }


  /**
   * @brief Comparison operator.
   *
   * This operation is only defined if either D2
   * is convertible to D1 or vice versa. If that is
   * not the case the compiler will report an error
   * as EnableIfInterOperable<D1,D2,bool>::type is
   * not defined.
   *
   */
  template<class T1, class V1, class R1, class D,
      class T2, class V2, class R2>
  inline typename EnableIfInterOperable<T1,T2,bool>::type
  operator<=(const RandomAccessIteratorFacade<T1,V1,R1,D>& lhs,
             const RandomAccessIteratorFacade<T2,V2,R2,D>& rhs)
  {
    if(std::is_convertible<T2,T1>::value)
      return static_cast<const T1&>(lhs).distanceTo(static_cast<const T2&>(rhs))>=0;
    else
      return static_cast<const T2&>(rhs).distanceTo(static_cast<const T1&>(lhs))<=0;
  }


  /**
   * @brief Comparison operator.
   *
   * This operation is only defined if either D2
   * is convertible to D1 or vice versa. If that is
   * not the case the compiler will report an error
   * as EnableIfInterOperable<D1,D2,bool>::type is
   * not defined.
   *
   */
  template<class T1, class V1, class R1, class D,
      class T2, class V2, class R2>
  inline typename EnableIfInterOperable<T1,T2,bool>::type
  operator>(const RandomAccessIteratorFacade<T1,V1,R1,D>& lhs,
            const RandomAccessIteratorFacade<T2,V2,R2,D>& rhs)
  {
    if(std::is_convertible<T2,T1>::value)
      return static_cast<const T1&>(lhs).distanceTo(static_cast<const T2&>(rhs))<0;
    else
      return static_cast<const T2&>(rhs).distanceTo(static_cast<const T1&>(lhs))>0;
  }

  /**
   * @brief Comparison operator.
   *
   * This operation is only defined if either D2
   * is convertible to D1 or vice versa. If that is
   * not the case the compiler will report an error
   * as EnableIfInterOperable<D1,D2,bool>::type is
   * not defined.
   *
   */
  template<class T1, class V1, class R1, class D,
      class T2, class V2, class R2>
  inline typename EnableIfInterOperable<T1,T2,bool>::type
  operator>=(const RandomAccessIteratorFacade<T1,V1,R1,D>& lhs,
             const RandomAccessIteratorFacade<T2,V2,R2,D>& rhs)
  {
    if(std::is_convertible<T2,T1>::value)
      return static_cast<const T1&>(lhs).distanceTo(static_cast<const T2&>(rhs))<=0;
    else
      return static_cast<const T2&>(rhs).distanceTo(static_cast<const T1&>(lhs))>=0;
  }

  /**
   * @brief Calculates the difference between two pointers.
   *
   * This operation is only defined if either D2
   * is convertible to D1 or vice versa. If that is
   * not the case the compiler will report an error
   * as EnableIfInterOperable<D1,D2,bool>::type is
   * not defined.
   *
   */
  template<class T1, class V1, class R1, class D,
      class T2, class V2, class R2>
  inline typename EnableIfInterOperable<T1,T2,D>::type
  operator-(const RandomAccessIteratorFacade<T1,V1,R1,D>& lhs,
            const RandomAccessIteratorFacade<T2,V2,R2,D>& rhs)
  {
    if(std::is_convertible<T2,T1>::value)
      return -static_cast<const T1&>(lhs).distanceTo(static_cast<const T2&>(rhs));
    else
      return static_cast<const T2&>(rhs).distanceTo(static_cast<const T1&>(lhs));
  }


  /**
   * \brief Helper to mimic a pointer for proxy objects.
   *
   * This class is intended to be used as return type
   * for operator-> on an iterator using proxy-values.
   * It stores the proxy value and forwards operator->
   * to the address of this value. In particular
   * it can be passed to IteratorFacade when creating
   * proxy iterators.
   */
  template<class ProxyType>
  class ProxyArrowResult
  {
  public:
    constexpr ProxyArrowResult(ProxyType&& p)
      noexcept(std::is_nothrow_constructible_v<ProxyType, ProxyType&&>)
      : p_(std::move(p))
    {}

    constexpr ProxyArrowResult(const ProxyType& p)
      noexcept(std::is_nothrow_constructible_v<ProxyType, const ProxyType&>)
      : p_(p)
    {}

    constexpr const ProxyType* operator->() const noexcept
    {
      return &p_;
    }

    constexpr ProxyType* operator->() noexcept
    {
      return &p_;
    }

  private:
    ProxyType p_;
  };


  /**
   * \brief This class encapsulates access of IteratorFacade
   *
   * If you derive from IteratorFacade and want to provide
   * iterator operations using `it.baseIterator()`, then
   * this method either has to be public or IteratorFacadeAccess
   * must be declared a friend.
   */
  struct IteratorFacadeAccess
  {

    //! @private
    template<class It>
    static constexpr auto baseIterator(It&& it) noexcept
      -> decltype(it.baseIterator())
    {
      return it.baseIterator();
    }

    //! @private
    template<class It>
    static constexpr auto derived(It&& it) noexcept
      -> decltype(it.derived())
    {
      return it.derived();
    }

  };



  namespace Impl::Concepts {

    using namespace Dune::Concept;

    template<class R>
    struct BaseIterDereferenceOp
    {
      template<class It>
      auto require(const It& it) -> decltype(
        *(IteratorFacadeAccess::baseIterator(it)),
        requireConvertible<decltype(*(IteratorFacadeAccess::baseIterator(it))), R>()
      );
    };

    struct IterEqualsOp
    {
      template<class It1, class It2>
      auto require(const It1& it1, const It2& it2) -> decltype(
        requireConvertible<bool>(it1 == it2)
      );
    };

    struct BaseIterEqualsOp
    {
      template<class It1, class It2>
      auto require(const It1& it1, const It2& it2) -> decltype(
        Dune::Concept::requireConvertible<bool>(IteratorFacadeAccess::baseIterator(it1) == IteratorFacadeAccess::baseIterator(it2))
      );
    };

    struct BaseIterIncrementOp
    {
      template<class It>
      auto require(It it) -> decltype(
        ++(IteratorFacadeAccess::baseIterator(it))
      );
    };

    struct BaseIterDecrementOp
    {
      template<class It>
      auto require(It it) -> decltype(
        --(IteratorFacadeAccess::baseIterator(it))
      );
    };

    template<class D>
    struct IterAdvanceOp
    {
      template<class It>
      auto require(It it) -> decltype(
        it += std::declval<D>()
      );
    };

    template<class D>
    struct BaseIterAdvanceOp
    {
      template<class It>
      auto require(It it) -> decltype(
        IteratorFacadeAccess::baseIterator(it) += std::declval<D>()
      );
    };

    template<class D>
    struct IterDistanceOp
    {
      template<class It1, class It2>
      auto require(const It1& it1, const It2& it2) -> decltype(
        Dune::Concept::requireConvertible<D>(it1 - it2)
      );
    };

    template<class D>
    struct BaseIterDistanceOp
    {
      template<class It1, class It2>
      auto require(const It1& it1, const It2& it2) -> decltype(
        Dune::Concept::requireConvertible<D>(IteratorFacadeAccess::baseIterator(it1) - IteratorFacadeAccess::baseIterator(it2))
      );
    };

  } // namespace Impl::Concept



  /**
   * @brief CRTP-Mixing class for stl conformant iterators of given iterator category
   *
   * The iterator category is given by the corresponding tag class.
   * Currently supported tags are `std::forward_iterator_tag`,
   * `std::bidirectional_iterator_tag`, `std::random_access_iterator_tag`.
   *
   * For proxy iterators (i.e. iterator that don't return a real reference but
   * a so called proxy-value that behaves like a reference), the template parameter
   * `R` should be the type of the proxy-value and no reference. In the latter case
   * one should also use `P=ProxyArrowResult<R>` as pointer type used as return value
   * of `operator->`. If `P` is not a raw pointer type, then it must be constructable
   * from `V`.
   *
   * The derived class should implement methods as documented in the following.
   * Notice that, if the iterator provides multiple of the possible
   * implementations for a certain feature, then precedence for the
   * different implementation follows the order given below.
   *
   * For a forward iterator the derived class `It` must provide:
   *
   * * Dereferencing a const iterator using any of the following approaches:
   *   1. implement `*it`
   *   2. implement `*(it.baseIterator())`
   * * Incrementing a non-const iterator using any of the following approaches:
   *   1. implement `++it`
   *   2. implement `++(it.baseIterator())`
   *   3. implement `it+=1`
   * * Equality comparison of two const iterators using any of the following approaches:
   *   1. implement `it1==it2`
   *   2. implement `it1.baseIterator()==it2.baseIterator()`
   *
   * For a bidirectional iterator it must additionally provide:
   *
   * * Decrementing a non-const iterator using any of the following approaches:
   *   1. implement `--it`
   *   2. implement `--(it.baseIterator())`
   *   3. implement `it-=1`
   *
   * For a random access iterator it must additionally provide:
   *
   * * Advacing a non-const iterator by an offset using any of the following approaches:
   *   1. implement `it+=n`
   *   2. implement `it.baseIterator()+=n`
   * * Computing the distance between two const iterators using any of the following approaches:
   *   1. implement `it1-it2`
   *   2. implement `it1.baseIterator()-it2.baseIterator()`
   *
   * When relying on option 2 for any of those features, the `it.baseIterator()`
   * method can be made private to hide it from the user. Then the derived
   * class must declare IteratorFacadeAccess as friend. Notice that depending
   * on the feature it is used for, `it.baseIterator()` must be a const or non-const
   * method. Thus the derived class must provide both versions if it wants
   * to implement const and non-const operation in terms of `it.baseIterator().
   *
   * For example a forward iterator for values of type `V` could be implemented
   * by providing the core operations manually (option 1 above):
   *
   * @code
   * class FooIterator
   *   : public Dune::IteratorFacade<FooIterator, std::forward_iterator_tag, V>
   * {
   *   using Facade = Dune::IteratorFacade<FooIterator, std::forward_iterator_tag, V>;
   *
   * public:
   *
   *   using reference = Facade::reference;
   *
   *   reference operator*() const
   *   { return [implement dereferencing here]; }
   *
   *   FooIterator& operator++() const
   *   { [implement incrementing here]; return *this; }
   *
   *   friend bool operator==(const FooIterator& it1, const FooIterator& it2)
   *   { return [implement comparison here]; }
   * };
   * @endcode
   *
   * Alternatively the iterator can delegate arithmetic operations and
   * comparisons to an underlying iterator/pointer/number (option 2 above).
   * E.g. a random access iterator where the iterator position
   * is identified by a consecutive number can be implemented
   * as:
   *
   * @code
   * class BarIterator
   *   : public Dune::IteratorFacade<BarIterator, std::random_access_iterator_tag, V>
   * {
   *   using Facade = Dune::IteratorFacade<BarIterator, std::random_access_iterator_tag, V>;
   *
   * public:
   *
   *   using reference = Facade::reference;
   *   using difference_type = Facade::difference_type;
   *
   *   // Only implement dereferencing manually
   *   reference operator*() const
   *   { return [implement dereferencing at current position p_ here]; }
   *
   * private:
   *
   *   // Delegate arithmetic operations and comparisons to p_ by exporting
   *   // it in const and mutable form using baseIterator().
   *   difference_type& baseIterator() { return p_; }
   *   const difference_type& baseIterator() const { return p_; }
   *
   *   // Grant access to the private baseIterator() by a friend declaration.
   *   friend Dune::IteratorFacadeAccess;
   *
   *   difference_type p_;
   * };
   * @endcode
   *
   * When providing `baseIterator()` individual method can still be overloaded
   * by implementing them manually.
   * E.g. a random access iterator
   * for values of type `V` that returns reference-like proxy objects of type
   * `R` instead of plain `V&` references and relies on an underlying iterator
   * except for equality comparison can be implemented as:
   *
   * @code
   * class ProxyIterator
   *   : public Dune::IteratorFacade<ProxyIterator, std::random_access_iterator_tag, V, R, Dune::ProxyArrowResult<R>>
   * {
   *   using Facade = Dune::IteratorFacade<ProxyIterator, std::random_access_iterator_tag, V, R, Dune::ProxyArrowResult<R>>;
   *
   * public:
   *
   *   using reference = Facade::reference;
   *
   *   // Dereferencing yields copies of type R=reference
   *   reference operator*() const
   *   { return [implement dereferencing at current position it_ here]; }
   *
   *   // Override comparison manually here
   *   friend bool operator==(const ProxyIterator& it1, const ProxyIterator& it2)
   *   { return [implement custom comparison here]; }
   *
   * private:
   *
   *   // Delegate arithmetic operations to underlying base iterator.
   *   BaseIterator& baseIterator() { return it_; }
   *   const BaseIterator& baseIterator() const { return it_; }
   *   friend Dune::IteratorFacadeAccess;
   *
   *   BaseIterator it_;
   * };
   * @endcode
   *
   * \tparam It The derived iterator class
   * \tparam C Tag class of iterator category
   * \tparam V The value type
   * \tparam R The reference type, defaults to V&
   * \tparam P Pointer type, defaults to V*
   * \tparam D The type for differences between two iterators, defaults to std::ptrdiff_t
   */
  template<class It, class C, class V, class R = V&, class P = V*, class D = std::ptrdiff_t>
  class IteratorFacade
  {
    static constexpr bool isBidirectional = std::is_convertible_v<C, std::bidirectional_iterator_tag>;
    static constexpr bool isRandomAccess = std::is_convertible_v<C, std::random_access_iterator_tag>;

    // We make IteratorFacadeAccess a friend to allow forwarding of the derived()
    // methods to the free operators instead of havin to do raw casts there.
    // This allows to encapsulate all casts within IteratorFacade itself.
    friend IteratorFacadeAccess;

  protected:

    //! The derived iterator type
    using DerivedIterator = It;

    //! Cast of `*this` to const DerivedIterator type
    constexpr const DerivedIterator& derived() const
    {
      return static_cast<const DerivedIterator&>(*this);
    }

    //! Cast of `*this` to DerivedIterator type
    constexpr DerivedIterator& derived()
    {
      return static_cast<DerivedIterator&>(*this);
    }

  public:

    // Standard types of of C++ iterators
    using iterator_category = C;
    using value_type = typename std::remove_const<V>::type;
    using reference = R;
    using pointer = P;
    using difference_type = D;

    // Corresponding Dune typedefs
    using Value = value_type;
    using Reference = reference;
    using Pointer = pointer;
    using DifferenceType = difference_type;

    // Only defined to do static assertions.
    IteratorFacade()
    {
      static_assert(std::is_signed_v<difference_type>,
        "Type used as difference_type must be signed");
      const DerivedIterator& constDerived = derived();
      static_assert(std::is_convertible_v<decltype(*constDerived), reference>,
        "Derived class does not implement `*it` or `*(it.baseIterator())` for const `it` required by IteratorFacade<..., std::forward_iterator_tag, ...>.");
      static_assert(std::is_convertible_v<decltype(++derived()), DerivedIterator&>,
        "Derived class does not implement `++it`, `++(it.baseIterator())`, or `it+=1` for mutable `it` required by IteratorFacade<..., std::forward_iterator_tag, ...>.");
      static_assert(std::is_convertible_v<decltype(constDerived==constDerived), bool>,
        "Derived class does not implement `it1==it2` or `it1.baseIterator()==it2.baseIterator()` for const `it1` and `it2` required by IteratorFacade<..., std::forward_iterator_tag, ...>.");
      if constexpr (isBidirectional)
        static_assert(std::is_convertible_v<decltype(--derived()), DerivedIterator&>,
          "Derived class does not implement `--it`, `--(it.baseIterator())`, or `it-=1` for mutable `it` required by IteratorFacade<..., std::bidirectional_iterator_tag, ...>.");
      if constexpr (isRandomAccess)
      {
        static_assert(std::is_convertible_v<decltype(derived()+=std::declval<difference_type>()), DerivedIterator&>,
          "Derived class does not implement `it+=` or `it.baseIterator()+=` for mutable `it` required by IteratorFacade<..., std::random_access_iterator_tag, ...>.");
        static_assert(std::is_convertible_v<decltype(constDerived-constDerived), difference_type>,
          "Derived class does not implement `it1-it2` or `it1.baseIterator()-it2.baseIterator()` for const `it1` and `it2` required by IteratorFacade<..., std::random_access_iterator_tag, ...>.");
      }
    }

    /** @brief Dereferencing operator. */
    constexpr decltype(auto) operator*() const
    {
      if constexpr (Dune::models<Impl::Concepts::BaseIterDereferenceOp<reference>, DerivedIterator>())
        return *(IteratorFacadeAccess::baseIterator(derived()));
      else
        static_assert(AlwaysFalse<It>::value,
          "Derived class does not implement `*it` or `*(it.baseIterator())` for const `it` required by IteratorFacade<..., std::forward_iterator_tag, ...>.");
    }

    /** @brief Arrow access to members of referenced value. */
    constexpr pointer operator->() const
    {
      if constexpr (std::is_pointer_v<pointer>)
        return std::addressof(*derived());
      else
        return pointer(*derived());
    }

    /** @brief Preincrement operator. */
    constexpr decltype(auto) operator++()
    {
      if constexpr (Dune::models<Impl::Concepts::BaseIterIncrementOp, DerivedIterator>())
      {
        ++(IteratorFacadeAccess::baseIterator(derived()));
        return derived();
      }
      else if constexpr (Dune::models<Impl::Concepts::IterAdvanceOp<difference_type>, DerivedIterator>())
      {
        derived() += 1;
        return derived();
      }
      else
        static_assert(AlwaysFalse<It>::value,
          "Derived class does not implement `++it`, `++(it.baseIterator())`, or `it+=1` for mutable `it` required by IteratorFacade<..., std::forward_iterator_tag, ...>.");
    }

    /** @brief Postincrement operator. */
    constexpr DerivedIterator operator++(int)
    {
      DerivedIterator tmp(derived());
      this->operator++();
      return tmp;
    }

    /**
     * @brief Predecrement operator.
     *
     * Only enabled for bidirectional and random-access iterators.
     */
    template<bool dummy=true, std::enable_if_t<isBidirectional and dummy, int> =0>
    constexpr decltype(auto) operator--()
    {
      if constexpr (Dune::models<Impl::Concepts::BaseIterDecrementOp, DerivedIterator>())
      {
        --(IteratorFacadeAccess::baseIterator(derived()));
        return derived();
      }
      else if constexpr (Dune::models<Impl::Concepts::IterAdvanceOp<difference_type>, DerivedIterator>())
      {
        derived() -= 1;
        return derived();
      }
      else
        static_assert(AlwaysFalse<It>::value,
          "Derived class does not implement `--it`, `--(it.baseIterator())`, or `it-=1` for mutable `it` required by IteratorFacade<..., std::bidirectional_iterator_tag, ...>.");
    }

    /**
     * @brief Postdecrement operator.
     *
     * Only enabled for bidirectional and random-access iterators.
     */
    template<bool dummy=true, std::enable_if_t<isBidirectional and dummy, int> =0>
    constexpr DerivedIterator operator--(int)
    {
      DerivedIterator tmp(derived());
      this->operator--();
      return tmp;
    }

    /**
     * @brief Dereference element with given offset form this iterator
     * @param n The distance to the element.
     * @return The element at that distance.
     *
     * Only enabled for random-access iterators.
     */
    template<bool dummy=true, std::enable_if_t<isRandomAccess and dummy, int> =0>
    constexpr reference operator[](difference_type n) const
    {
      return *(derived()+n);
    }

    /**
     * @brief Increment iterator by given value
     *
     * Only enabled for random-access iterators.
     */
    template<bool dummy=true, std::enable_if_t<isRandomAccess and dummy, int> =0>
    constexpr decltype(auto) operator+=(difference_type n)
    {
      if constexpr (Dune::models<Impl::Concepts::BaseIterAdvanceOp<difference_type>, DerivedIterator>())
      {
        IteratorFacadeAccess::baseIterator(derived()) += n;
        return derived();
      }
      else
        static_assert(AlwaysFalse<It>::value,
          "Derived class does not implement `it+=` or `it.baseIterator()+=` for mutable `it` required by IteratorFacade<..., std::random_access_iterator_tag, ...>.");
    }

    /**
     * @brief Create iterator incremented by given value
     *
     * Only enabled for random-access iterators.
     */
    template<bool dummy=true, std::enable_if_t<isRandomAccess and dummy, int> =0>
    constexpr DerivedIterator operator+(difference_type n) const
    {
      DerivedIterator tmp(derived());
      tmp += n;
      return tmp;
    }

    /**
     * @brief Create iterator incremented by given value
     *
     * Only enabled for random-access iterators.
     */
    template<bool dummy=true, std::enable_if_t<isRandomAccess and dummy, int> =0>
    friend constexpr DerivedIterator operator+(difference_type n, const IteratorFacade& it)
    {
      DerivedIterator tmp(it.derived());
      tmp += n;
      return tmp;
    }

    /**
     * @brief Decrement iterator by given value
     *
     * Only enabled for random-access iterators.
     */
    template<bool dummy=true, std::enable_if_t<isRandomAccess and dummy, int> =0>
    constexpr DerivedIterator& operator-=(difference_type n)
    {
      derived() += (-n);
      return derived();
    }

    /**
     * @brief Create iterator decremented by given value
     *
     * Only enabled for random-access iterators.
     */
    template<bool dummy=true, std::enable_if_t<isRandomAccess and dummy, int> =0>
    constexpr DerivedIterator operator-(difference_type n) const
    {
      DerivedIterator tmp(derived());
      tmp -= n;
      return tmp;
    }

  };



  /**
   * @brief Equality comparison for IteratorFacade
   *
   * This operation is defined if the derived
   * iterator classes T1 and T2 are interoperable, i.e.
   * if T1 is convertible to T2 or vice versa and provide
   * `it1.equals(t2)` or `it2.equals(t1)`. Alternatively they
   * may provide `it1.baseIterator() == it2.baseIterator()`
   * for two const iterators.
   */
  template<class T1, class T2, class C, class V1, class V2, class R1, class R2, class P1, class P2, class D1, class D2>
  constexpr auto operator==(const IteratorFacade<T1,C,V1,R1,P1,D1>& it1, const IteratorFacade<T2,C,V2,R2,P2,D2>& it2)
  {
    const T1& derivedIt1 = IteratorFacadeAccess::derived(it1);
    const T2& derivedIt2 = IteratorFacadeAccess::derived(it2);
    if constexpr (Dune::models<Impl::Concepts::BaseIterEqualsOp, T1, T2>())
      return IteratorFacadeAccess::baseIterator(derivedIt1) == IteratorFacadeAccess::baseIterator(derivedIt2);
    else
      static_assert(AlwaysFalse<T1>::value,
        "Derived class does not implement `it1==it2` or `it1.baseIterator()==it2.baseIterator()` for const `it1` and `it2` required by IteratorFacade<..., std::forward_iterator_tag, ...>.");
  }

  /**
   * @brief Inequality comparison for IteratorFacade
   *
   * This operation is implemented as `not(it1==it2)` if the
   * passed iterators support this operation (cf. documentation
   * of `operator==`).
   */
  template<class T1, class T2, class C, class V1, class V2, class R1, class R2, class P1, class P2, class D1, class D2,
    std::enable_if_t< Dune::models<Impl::Concepts::IterEqualsOp,T1, T2>() , int> =0>
  constexpr bool operator!=(const IteratorFacade<T1,C,V1,R1,P1,D1>& it1, const IteratorFacade<T2,C,V2,R2,P2,D2>& it2)
  {
    const T1& derivedIt1 = IteratorFacadeAccess::derived(it1);
    const T2& derivedIt2 = IteratorFacadeAccess::derived(it2);
    return not(derivedIt1 == derivedIt2);
  }

  /**
   * @brief Difference for two IteratorFacade objects
   *
   * This operation is defined if the derived
   * iterator classes T1 and T2 are interoperable, i.e.
   * if T1 is convertible to T2 or vice versa and provide
   * `it1.distanceTo(t2)` or `it2.distanceTo(t1)`. Alternatively they
   * may provide `it1.baseIterator() - it2.baseIterator()`
   * for two const iterators.
   */
  template<class T1, class T2, class C, class V1, class V2, class R1, class R2, class P1, class P2, class D>
  constexpr auto operator-(const IteratorFacade<T1,C,V1,R1,P1,D>& it1, const IteratorFacade<T2,C,V2,R2,P2,D>& it2)
  {
    const T1& derivedIt1 = IteratorFacadeAccess::derived(it1);
    const T2& derivedIt2 = IteratorFacadeAccess::derived(it2);
    if constexpr (Dune::models<Impl::Concepts::BaseIterDistanceOp<D>,T1, T2>())
      return D(IteratorFacadeAccess::baseIterator(derivedIt1) - IteratorFacadeAccess::baseIterator(derivedIt2));
    else
      static_assert(AlwaysFalse<T1>::value,
        "Derived class does not implement `it1-it2` or `it1.baseIterator()-it2.baseIterator()` for const `it1` and `it2` required by IteratorFacade<..., std::random_access_iterator_tag, ...>.");
  }

  /**
   * @brief Comparison for IteratorFacade
   *
   * This operation is implemented as `(it1-it2)<0` if the
   * passed iterators support this operation (cf. documentation
   * of `operator-`).
   */
  template<class T1, class T2, class C, class V1, class V2, class R1, class R2, class P1, class P2, class D1, class D2,
    std::enable_if_t< Dune::models<Impl::Concepts::IterDistanceOp<D1>,T1, T2>() , int> =0>
  constexpr bool operator<(const IteratorFacade<T1,C,V1,R1,P1,D1>& it1, const IteratorFacade<T2,C,V2,R2,P2,D2>& it2)
  {
    const T1& derivedIt1 = IteratorFacadeAccess::derived(it1);
    const T2& derivedIt2 = IteratorFacadeAccess::derived(it2);
    return (derivedIt1 - derivedIt2) < D1(0);
  }

  /**
   * @brief Comparison for IteratorFacade
   *
   * This operation is implemented as `(it1-it2)<=0` if the
   * passed iterators support this operation (cf. documentation
   * of `operator-`).
   */
  template<class T1, class T2, class C, class V1, class V2, class R1, class R2, class P1, class P2, class D1, class D2,
    std::enable_if_t< Dune::models<Impl::Concepts::IterDistanceOp<D1>,T1, T2>() , int> =0>
  constexpr bool operator<=(const IteratorFacade<T1,C,V1,R1,P1,D1>& it1, const IteratorFacade<T2,C,V2,R2,P2,D2>& it2)
  {
    const T1& derivedIt1 = IteratorFacadeAccess::derived(it1);
    const T2& derivedIt2 = IteratorFacadeAccess::derived(it2);
    return (derivedIt1 - derivedIt2) <= D1(0);
  }

  /**
   * @brief Comparison for IteratorFacade
   *
   * This operation is implemented as `(it1-it2)>0` if the
   * passed iterators support this operation (cf. documentation
   * of `operator-`).
   */
  template<class T1, class T2, class C, class V1, class V2, class R1, class R2, class P1, class P2, class D1, class D2,
    std::enable_if_t< Dune::models<Impl::Concepts::IterDistanceOp<D1>,T1, T2>() , int> =0>
  constexpr bool operator>(const IteratorFacade<T1,C,V1,R1,P1,D1>& it1, const IteratorFacade<T2,C,V2,R2,P2,D2>& it2)
  {
    const T1& derivedIt1 = IteratorFacadeAccess::derived(it1);
    const T2& derivedIt2 = IteratorFacadeAccess::derived(it2);
    return (derivedIt1 - derivedIt2) > D1(0);
  }

  /**
   * @brief Comparison for IteratorFacade
   *
   * This operation is implemented as `(it1-it2)>=0` if the
   * passed iterators support this operation (cf. documentation
   * of `operator-`).
   */
  template<class T1, class T2, class C, class V1, class V2, class R1, class R2, class P1, class P2, class D1, class D2,
    std::enable_if_t< Dune::models<Impl::Concepts::IterDistanceOp<D1>,T1, T2>() , int> =0>
  constexpr bool operator>=(const IteratorFacade<T1,C,V1,R1,P1,D1>& it1, const IteratorFacade<T2,C,V2,R2,P2,D2>& it2)
  {
    const T1& derivedIt1 = IteratorFacadeAccess::derived(it1);
    const T2& derivedIt2 = IteratorFacadeAccess::derived(it2);
    return (derivedIt1 - derivedIt2) >= D1(0);
  }



  /** @} */
}
#endif
