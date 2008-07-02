// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#include "config.h"

#include <dune/common/smartpointer.hh>
#include <iostream>
#include <vector>
#include <cstdlib>

int main(){
  using namespace Dune;
  int ret=0;
  {
    SmartPointer<double> b;
    {
      SmartPointer<double> d(b);
      *b = 7;
    }
    if(b.count()!=1) {
      std::cout << "Reference count is wrong! "<<__LINE__<<":"<<
      __FILE__<<std::endl;
      ret=1;
    }
    {
      SmartPointer<double> c(b);

      if(*b!=*c) {
        std::cerr<<"References do not match! "<<__LINE__<<":"<<
        __FILE__<<std::endl;
        ret=1;
      }
      if(b.count()!=2 || c.count()!=2) {
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

    if(b.count()!=1) {
      std::cout << "Reference count is wrong! "<<__LINE__<<":"<<
      __FILE__<<std::endl;
      ret=1;
    }
  }
  exit(ret);
}
