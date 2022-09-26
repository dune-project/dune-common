// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_GENERICITERATOR_HH
#define DUNE_GENERICITERATOR_HH

#include <dune/common/iteratorfacades.hh>
#include <cassert>

namespace Dune {

  /*! \defgroup GenericIterator GenericIterator
     \ingroup IteratorFacades

     \brief Generic Iterator class for writing stl conformant iterators
     for any container class with operator[]

     Using this template class you can create an iterator and a const_iterator
     for any container class.

     Imagine you have SimpleContainer and would like to have an iterator.
     All you have to do is provide operator[], begin() and end()
     (for const and for non-const).

     \code
     template<class T>
     class SimpleContainer{
     public:
      typedef GenericIterator<SimpleContainer<T>,T> iterator;

      typedef GenericIterator<const SimpleContainer<T>,const T> const_iterator;

      SimpleContainer(){
        for(int i=0; i < 100; i++)
          values_[i]=i;
      }

      iterator begin(){
        return iterator(*this, 0);
      }

      const_iterator begin() const{
        return const_iterator(*this, 0);
      }

      iterator end(){
        return iterator(*this, 100);
      }

      const_iterator end() const{
        return const_iterator(*this, 100);
      }

      T& operator[](int i){
        return values_[i];
      }

      const T& operator[](int i) const{
        return values_[i];
      }
     private:
      T values_[100];
     };
     \endcode

     See dune/common/test/iteratorfacestest.hh for details or
     Dune::QuadratureDefault in dune/quadrature/quadrature.hh
     for a real example.
   */

  /**
   * @file
   * @brief Implements a generic iterator class for writing stl conformant iterators.
   *
   * Using this generic iterator writing iterators for containers
   * that implement operator[] is only a matter of seconds.
   */

  /**
     \brief Get the 'const' version of a reference to a mutable object

     Given a reference R=T& const_reference<R>::type gives you the typedef for const T&
   */
  template<class R>
  struct const_reference
  {
    typedef const R type;
  };

  template<class R>
  struct const_reference<const R>
  {
    typedef const R type;
  };

  template<class R>
  struct const_reference<R&>
  {
    typedef const R& type;
  };

  template<class R>
  struct const_reference<const R&>
  {
    typedef const R& type;
  };

  /**
     \brief get the 'mutable' version of a reference to a const object

     given a const reference R=const T& mutable_reference<R>::type gives you the typedef for T&
   */
  template<class R>
  struct mutable_reference
  {
    typedef R type;
  };

  template<class R>
  struct mutable_reference<const R>
  {
    typedef R type;
  };

  template<class R>
  struct mutable_reference<R&>
  {
    typedef R& type;
  };

  template<class R>
  struct mutable_reference<const R&>
  {
    typedef R& type;
  };

  /** @addtogroup GenericIterator
   *
   * @{
   */

  /**
   * @brief Generic class for stl-conforming iterators for container classes with operator[].
   *
   * If template parameter C has a const qualifier we are a const iterator, otherwise we
   * are a mutable iterator.
   */
  template<class C, class T, class R=T&, class D = std::ptrdiff_t,
      template<class,class,class,class> class IteratorFacade=RandomAccessIteratorFacade>
  class GenericIterator :
    public IteratorFacade<GenericIterator<C,T,R,D,IteratorFacade>,T,R,D>
  {
    friend class GenericIterator<typename std::remove_const<C>::type, typename std::remove_const<T>::type, typename mutable_reference<R>::type, D, IteratorFacade>;
    friend class GenericIterator<const typename std::remove_const<C>::type, const typename std::remove_const<T>::type, typename const_reference<R>::type, D, IteratorFacade>;

    typedef GenericIterator<typename std::remove_const<C>::type, typename std::remove_const<T>::type, typename mutable_reference<R>::type, D, IteratorFacade> MutableIterator;
    typedef GenericIterator<const typename std::remove_const<C>::type, const typename std::remove_const<T>::type, typename const_reference<R>::type, D, IteratorFacade> ConstIterator;

  public:

    /**
     * @brief The type of container we are an iterator for.
     *
     * The container type must provide an operator[] method.
     *
     * If C has a const qualifier we are a const iterator, otherwise we
     * are a mutable iterator.
     */
    typedef C Container;

    /**
     * @brief The value type of the iterator.
     *
     * This is the return type when dereferencing the iterator.
     */
    typedef T Value;

    /**
     * @brief The type of the difference between two positions.
     */
    typedef D DifferenceType;

    /**
     * @brief The type of the reference to the values accessed.
     */
    typedef R Reference;

    // Constructors needed by the base iterators
    GenericIterator() : container_(0), position_(0)
    {}

    /**
     * @brief Constructor
     * @param cont Reference to the container we are an iterator for
     * @param pos The position the iterator will be positioned to
     * (e.g. 0 for an iterator returned by Container::begin() or
     * the size of the container for an iterator returned by Container::end()
     */
    GenericIterator(Container& cont, DifferenceType pos)
      : container_(&cont), position_(pos)
    {}

    /**
     * @brief Copy constructor
     *
     * This is somehow hard to understand, therefore play with the cases:
     * 1. if we are mutable this is the only valid copy constructor, as the argument is a mutable iterator
     * 2. if we are a const iterator the argument is a mutable iterator => This is the needed conversion to initialize a const iterator from a mutable one.
     */
    GenericIterator(const MutableIterator& other) : container_(other.container_), position_(other.position_)
    {}

    /**
     * @brief Copy constructor
     *
     * @warning Calling this method results in a compiler error, if this is a mutable iterator.
     *
     * This is somehow hard to understand, therefore play with the cases:
     * 1. if we are mutable the arguments is a const iterator and therefore calling this method is mistake in the user's code and results in a (probably not understandable) compiler error
     * 2. If we are a const iterator this is the default copy constructor as the argument is a const iterator too.
     */
    GenericIterator(const ConstIterator& other) : container_(other.container_), position_(other.position_)
    {}

    // Methods needed by the forward iterator
    bool equals(const MutableIterator & other) const
    {
      return position_ == other.position_ && container_ == other.container_;
    }

    bool equals(const ConstIterator & other) const
    {
      return position_ == other.position_ && container_ == other.container_;
    }

    Reference dereference() const {
      return container_->operator[](position_);
    }

    void increment(){
      ++position_;
    }

    // Additional function needed by BidirectionalIterator
    void decrement(){
      --position_;
    }

    // Additional function needed by RandomAccessIterator
    Reference elementAt(DifferenceType i) const {
      return container_->operator[](position_+i);
    }

    void advance(DifferenceType n){
      position_=position_+n;
    }

    DifferenceType distanceTo(const MutableIterator& other) const
    {
      assert(other.container_==container_);
      return other.position_ - position_;
    }

    DifferenceType distanceTo(const ConstIterator& other) const
    {
      assert(other.container_==container_);
      return other.position_ - position_;
    }

  private:
    Container *container_;
    DifferenceType position_;
  };

  /** @} */

} // end namespace Dune

#endif
