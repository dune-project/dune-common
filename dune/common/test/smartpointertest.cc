// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dune/common/shared_ptr.hh>

#include <cassert>
#include <iostream>
#include <vector>
#include <cstdlib>

int main(){
  using namespace Dune;
  int ret=0;
  {
    // test default constructor
    shared_ptr<int> foo;

    // test cast-to-bool
    if (foo) {
      std::cout << "Default constructor doesn't create a NULL pointer!" << std::endl;
      ret=1;
    }

    // test constructor from a given pointer
    shared_ptr<double> bar(new double(43.0));
    assert(bar);

    // test reset()
    bar.reset();
    assert(!bar);

    // test reset(T*)
    bar.reset(new double(44.0));
    assert(bar);
    assert(bar.use_count()==1);

    // test get()
    double* barPtr = bar.get();
    assert(barPtr);

    // test constructor from a given pointer
    shared_ptr<double> b(new double(42.0));
    {
      shared_ptr<double> d(b);
      *b = 7;
    }

    if(b.use_count()!=1) {
      std::cout << "Reference count is wrong! "<<__LINE__<<":"<<
      __FILE__<<std::endl;
      ret=1;
    }
    {
      shared_ptr<double> c(b);

      if(*b!=*c) {
        std::cerr<<"References do not match! "<<__LINE__<<":"<<
        __FILE__<<std::endl;
        ret=1;
      }
      if(b.use_count()!=2 || c.use_count()!=2) {
        std::cout << "Reference count is wrong! "<<__LINE__<<":"<<
        __FILE__<<std::endl;
        ret=1;
      }
      *b=8;
      if(*b!=8 || *c!=8) {
        std::cout<<"Assigning new value failed! "<<__LINE__<<":"<<
        __FILE__<<std::endl;
        ret=1;
      }
    }

    if(b.use_count()!=1) {
      std::cout << "Reference count is wrong! "<<__LINE__<<":"<<
      __FILE__<<std::endl;
      ret=1;
    }
  }
  return (ret);
}
