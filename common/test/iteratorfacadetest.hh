// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ITERATORFACADETEST_HH__
#define __DUNE_ITERATORFACADETEST_HH__
#include <dune/common/iteratorfacades.hh>
#include <dune/common/genericiterator.hh>
#include <dune/common/typetraits.hh>

template<class T>
class TestContainer {
public:
  typedef GenericIterator<TestContainer<T>,T> iterator;

  typedef GenericIterator<const TestContainer<T>,const T> const_iterator;

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

#endif
