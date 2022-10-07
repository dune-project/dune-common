// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/test/iteratorfacadetest.hh>
#include <dune/common/test/iteratortest.hh>
#include <iostream>
#include <algorithm>

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

int main(){
  // Test the TestIterator;
  TestContainer<double, Dune::ForwardIteratorFacade> forwardcontainer;
  TestContainer<double, Dune::BidirectionalIteratorFacade> bidicontainer;
  TestContainer<double, Dune::RandomAccessIteratorFacade> randomcontainer;

  int ret=0;

  ret += containerTest(forwardcontainer);
  ret += containerTest(bidicontainer);
  ret += containerTest(randomcontainer);

  return (ret);
}
