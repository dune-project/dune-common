// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifndef DUNE_ITERATORFACADETEST_HH
#define DUNE_ITERATORFACADETEST_HH
#include <dune/common/iteratorfacades.hh>
#include <dune/common/genericiterator.hh>
#include <dune/common/typetraits.hh>

template<class T,
    template<class,class,class,class> class IteratorFacade=Dune::RandomAccessIteratorFacade>
class TestContainer {
public:
  typedef Dune::GenericIterator<TestContainer<T,IteratorFacade>,T,T&,std::ptrdiff_t,IteratorFacade> iterator;

  typedef Dune::GenericIterator<const TestContainer<T,IteratorFacade>,const T,const T&,std::ptrdiff_t,IteratorFacade> const_iterator;

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
