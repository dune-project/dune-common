// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>

#include <dune/common/gcd.hh>
#include <dune/common/lcm.hh>

void test()
{
  static_assert((Dune::Gcd<2*2*2*5*5*5*11, 2*2*5*13>::value == 2*2*5),
                "gcd not working properly");
  static_assert((Dune::Lcm<11,3>::value == 33),
                "lcm not working properly");
  static_assert((Dune::Lcm<18,15>::value == 18*15/3),
                "lcm not working properly");
  static_assert((Dune::Lcm<10800,Dune::Lcm<36000,7680>::value>::value==1728000),
                "lcm not working properly");
}

int main()
{
  std::cout<<" gcd(2,5)="<<Dune::Gcd<2,5>::value<<" gcd(3, 18)="
           <<Dune::Gcd<3,18>::value<<" gcd("<<2*2*2*5*5*5*11<<", "
           << 2*2*5*13<<")="<<Dune::Gcd<2*2*2*5*5*5*11, 2*2*5*13>::value
           <<std::endl;
  std::cout<<" lcm(18,15)="<<Dune::Lcm<18,15>::value
           <<" lcm(10800,36000,7680)="<<Dune::Lcm<10800,Dune::Lcm<36000,7680>::value>::value<<std::endl;

}
