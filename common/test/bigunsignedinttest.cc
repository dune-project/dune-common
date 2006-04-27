// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "config.h"
#include <dune/common/bigunsignedint.hh>
#include <limits>
#include <iostream>

int main()
{
  std::cout<<"unsigned short: max="<<std::numeric_limits<unsigned short>::max()
           <<" min="<<std::numeric_limits<unsigned short>::min()
           <<" digits="<<std::numeric_limits<unsigned short>::digits<<std::endl;
  std::cout<<"int: max="<<std::numeric_limits<int>::max()<<" min="
           <<std::numeric_limits<int>::min()<<" digits="
           <<std::numeric_limits<int>::digits<<std::endl;
  std::cout<<"unsigned int: max="<<std::numeric_limits<unsigned int>::max()
           <<" min="<<std::numeric_limits<unsigned int>::min()<<" digits="
           <<std::numeric_limits<unsigned int>::digits<<" digits10="
           <<std::numeric_limits<unsigned int>::digits10<<" radix="
           <<std::numeric_limits<unsigned int>::radix<<" eps="
           <<std::numeric_limits<unsigned int>::epsilon()
           <<" round_error="
           <<std::numeric_limits<unsigned int>::round_error()
           <<" min_exponent="
           <<std::numeric_limits<unsigned int>::min_exponent
           <<" float_denorm_style="
           <<std::numeric_limits<unsigned int>::has_denorm
           <<std::endl;
  std::cout<<"bigunsignedint: max="<<std::numeric_limits<Dune::bigunsignedint<32> >::max()
           <<" min="<<std::numeric_limits<Dune::bigunsignedint<32> >::min()<<" digits="<<std::numeric_limits<Dune::bigunsignedint<32> >::digits<<std::endl;
  std::cout<<"bigunsignedint: max="<<std::numeric_limits<Dune::bigunsignedint<100> >::max()
           <<" min="<<std::numeric_limits<Dune::bigunsignedint<100> >::min()<<" digits="<<std::numeric_limits<Dune::bigunsignedint<100> >::digits<<std::endl;

}
