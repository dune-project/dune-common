// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <iostream>
#include <dune/common/fvector.hh>
#include <dune/common/fassign.hh>

using namespace Dune;

int main(int, char**) try
{
  Dune::FieldVector<double,3> pos;

  pos <<= 1, 0, 0;

}
catch (Exception e) {

  std::cout << e << std::endl;

}
