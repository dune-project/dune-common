// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_GENERICITERATOR_HH
#define DUNE_GENERICITERATOR_HH

#include <dune/common/iteratorfacades.hh>
#include <cassert>

namespace Dune {

  /*! \defgroup GenericIterator GenericIterator
     \ingroup IteratorFacades

     \brief Generic Iterator class for writing stl conformant iterators
     for any Container class with operator[]

     Using this template class you can create an iterator and a const_iterator
     for any Container class.

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
   * @brief This file implements a generic iterator classes for writing stl conformant iterators.
   *
   * With using this generic iterator writing iterators for containers
   * with operator[] is only a matter of seconds
   */

  /** @addtogroup GenericIterator
   *
   * @{
   */
  /**
   * @brief Generic class for stl conformant iterators for container classes with operator[].
   *
   */
  template<class C, class T>
  class GenericIterator :
    public Dune::RandomAccessIteratorFacade<GenericIterator<C,T>,T, T&, int>
  {
    friend class GenericIterator<typename Dune::RemoveConst<C>::Type, typename Dune::RemoveConst<T>::Type >;
    friend class GenericIterator<const typename Dune::RemoveConst<C>::Type, const typename Dune::RemoveConst<T>::Type >;

  public:

    // Constructors needed by the base iterators.
    GenericIterator() : container_(0), position_(0)
    {}

    GenericIterator(C& cont, int pos)
      : container_(&cont), position_(pos)
    {}

    GenericIterator(const GenericIterator<typename Dune::RemoveConst<C>::Type, typename Dune::RemoveConst<T>::Type >& other) : container_(other.container_), position_(other.position_)
    {}


    GenericIterator(const GenericIterator<const typename Dune::RemoveConst<C>::Type, const typename Dune::RemoveConst<T>::Type >& other) : container_(other.container_), position_(other.position_)
    {}

    // Methods needed by the forward iterator
    bool equals(const GenericIterator<typename Dune::RemoveConst<C>::Type,typename Dune::RemoveConst<T>::Type>& other) const
    {
      return position_ == other.position_ && container_ == other.container_;
    }


    bool equals(const GenericIterator<const typename Dune::RemoveConst<C>::Type,const typename Dune::RemoveConst<T>::Type>& other) const
    {
      return position_ == other.position_ && container_ == other.container_;
    }

    T& dereference() const {
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
    T& elementAt(int i) const {
      return container_->operator[](position_+i);
    }

    void advance(int n){
      position_=position_+n;
    }

    std::ptrdiff_t distanceTo(GenericIterator<const typename Dune::RemoveConst<C>::Type,const typename Dune::RemoveConst<T>::Type> other) const
    {
      assert(other.container_==container_);
      return other.position_ - position_;
    }

    std::ptrdiff_t distanceTo(GenericIterator<typename Dune::RemoveConst<C>::Type, typename Dune::RemoveConst<T>::Type> other) const
    {
      assert(other.container_==container_);
      return other.position_ - position_;
    }
  private:
    C *container_;
    size_t position_;
  };

  /** @} */

} // end namespace Dune

#endif
