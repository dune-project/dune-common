// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <dune/common/test/iteratorfacadetest.hh>
#include <dune/common/test/iteratortest.hh>
#include <dune/common/rangeutilities.hh>
#include <iostream>
#include <algorithm>
#include <vector>
#include <type_traits>

template<class Container>
void randomize(Container& cont){
  srand(300);
  double size=1000;

  for(int i=0; i < 100; i++) {
    cont[i] = (size*(rand()/(RAND_MAX+1.0)));

  }
}

template<class Container>
void print(Container& cont){
  for(int i=0; i < 100; i++)
    std::cout<<cont[i]<<" ";
  std::cout<<std::endl;
}

template<class Container>
int containerTest(Container & container)
{
  randomize(container);
  //  print(container);
  //std::sort(container.begin(), container.end());
  //print(container);

  const Container ccontainer(container);
  int ret=0;
  Printer<const double> print;
  ret += testIterator(container, print);
  ret += testIterator(ccontainer, print);

  return ret;
}



template<class C,
  class V = typename C::value_type,
  class R = std::conditional_t<std::is_const_v<C>, typename C::const_reference, typename C::reference>,
  class P = std::conditional_t<std::is_const_v<C>, typename C::const_pointer, typename C::pointer>
  >
class GenericIndexedIterator
  : public Dune::IteratorFacade<GenericIndexedIterator<C,V,R,P>, std::random_access_iterator_tag, V, R, P>
{
  using BaseIterator = Dune::Impl::IntegralRangeIterator<std::size_t>;
  using Facade = Dune::IteratorFacade<GenericIndexedIterator<C>, std::random_access_iterator_tag, V>;

  BaseIterator baseIt_;
  C* cPtr_;

public:

  GenericIndexedIterator() :
    baseIt_(),
    cPtr_(nullptr)
  {}

  GenericIndexedIterator(C& c, std::size_t i):
    baseIt_(i),
    cPtr_(&c)
  {}

  decltype(auto) operator*() const {
    return (*cPtr_)[index()];
  }

  std::size_t index() const {
    return *baseIt_;
  }

private:

  friend Dune::IteratorFacadeAccess;

  const BaseIterator& baseIterator() const {
    return baseIt_;
  }

  BaseIterator& baseIterator() {
    return baseIt_;
  }
};


template<class Container>
int checkGenericIndexedIterator(Container c)
{
  // Create copy sorted up to first and last entry
  auto cPartialSorted = c;
  {
    auto it = cPartialSorted.begin()+1;
    auto end = cPartialSorted.end()-1;
    std::sort(it, end);
  }
  int ret = 0;
  {
    auto it = GenericIndexedIterator(c, 1);
    auto end = GenericIndexedIterator(c, c.size()-1);
    std::sort(it, end);
    ret += (c != cPartialSorted);
  }
  {
    auto it = GenericIndexedIterator(c, 1);
    auto end = GenericIndexedIterator(c, c.size()-1);
    auto op = [](auto&&){};
    ret += testIterator(it, end, op, std::random_access_iterator_tag());
  }
  {
    auto it = GenericIndexedIterator(c, 1);
    auto end = GenericIndexedIterator(c, c.size()-1);
    std::size_t i = 1;
    for(; it!=end; ++it,++i)
      ret += (it.index() != i);
  }
  return ret;
}



int main(){
  // Test the TestIterator;
  TestContainer<double, Dune::ForwardIteratorFacade> forwardcontainer;
  TestContainer<double, Dune::BidirectionalIteratorFacade> bidicontainer;
  TestContainer<double, Dune::RandomAccessIteratorFacade> randomcontainer;

  int ret=0;

  ret += containerTest(forwardcontainer);
  ret += containerTest(bidicontainer);
  ret += containerTest(randomcontainer);

  ret += checkGenericIndexedIterator(std::vector<int>{5, 4, 3, 2, 1, 0});
  ret += checkGenericIndexedIterator(std::vector<bool>{true, true, true, false, false, false});

  return (ret);
}
