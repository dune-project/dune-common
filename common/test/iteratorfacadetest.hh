// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ITERATORFACADETEST_HH__
#define __DUNE_ITERATORFACADETEST_HH__
#include <dune/common/iteratorfacades.hh>
#include <dune/common/typetraits.hh>

template<class C, class T> class TestIterator;

template<class T>
class TestContainer {
  friend class TestIterator<TestContainer<T>,T>;
  friend class TestIterator<const TestContainer<T>, const T>;

  //  friend class TestConstIterator<T>;

public:
  typedef TestIterator<TestContainer<T>,T> iterator;

  typedef TestIterator<const TestContainer<T>,const T> const_iterator;

  TestContainer(){
    for(int i=0; i < 100; i++)
      values_[i]=i;
  }

  iterator begin(){
    return iterator(*this, 0);
  }

  const_iterator begin() const {
    return const_iterator(*this, 0);
  }

  iterator end(){
    return iterator(*this, 100);
  }

  const_iterator end() const {
    return const_iterator(*this, 100);
  }

  T& operator[](int i){
    return values_[i];
  }


  const T& operator[](int i) const {
    return values_[i];
  }
private:
  T values_[100];
};


template<class C, class T>
class TestIterator : public Dune::RandomAccessIteratorFacade<TestIterator<C,T>,T, T&, int>
{
  friend class TestIterator<typename Dune::RemoveConst<C>::Type, typename Dune::RemoveConst<T>::Type >;
  friend class TestIterator<const typename Dune::RemoveConst<C>::Type, const typename Dune::RemoveConst<T>::Type >;

public:

  // Constructors needed by the base iterators.
  TestIterator() : container_(0), position_(0)
  { }

  TestIterator(C& cont, int pos)
    : container_(&cont), position_(pos)
  {}

  TestIterator(const TestIterator<typename Dune::RemoveConst<C>::Type, typename Dune::RemoveConst<T>::Type >& other) : container_(other.container_), position_(other.position_)
  {}


  TestIterator(const TestIterator<const typename Dune::RemoveConst<C>::Type, const typename Dune::RemoveConst<T>::Type >& other) : container_(other.container_), position_(other.position_)
  {}

  // Methods needed by the forward iterator
  bool equals(const TestIterator<typename Dune::RemoveConst<C>::Type,typename Dune::RemoveConst<T>::Type>& other) const
  {
    return position_ == other.position_ && container_ == other.container_;
  }


  bool equals(const TestIterator<const typename Dune::RemoveConst<C>::Type,const typename Dune::RemoveConst<T>::Type>& other) const
  {
    return position_ == other.position_ && container_ == other.container_;
  }

  T& dereference() const {
    return container_->values_[position_];
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

  std::ptrdiff_t distanceTo(TestIterator<const typename Dune::RemoveConst<C>::Type,const typename Dune::RemoveConst<T>::Type> other) const
  {
    assert(other.container_==container_);
    return other.position_ - position_;
  }

  std::ptrdiff_t distanceTo(TestIterator<typename Dune::RemoveConst<C>::Type, typename Dune::RemoveConst<T>::Type> other) const
  {
    assert(other.container_==container_);
    return other.position_ - position_;
  }
private:
  C *container_;
  size_t position_;
};
#endif
