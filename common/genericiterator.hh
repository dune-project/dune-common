// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_GENERICITERATOR_HH
#define DUNE_GENERICITERATOR_HH

#include <dune/common/iteratorfacades.hh>

template<class C, class T>
class GenericIterator : public Dune::RandomAccessIteratorFacade<GenericIterator<C,T>,T, T&, int>
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

#endif
