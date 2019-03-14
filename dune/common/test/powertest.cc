// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include <iostream>

#include <dune/common/exceptions.hh>
#include <dune/common/power.hh>

using namespace Dune;

int main (int argc, char** argv) try
{
  if (Power<0>::eval(4) != 1)
    DUNE_THROW(MathError, "Power implementation does not compute the correct result");

  if (Power<1>::eval(4) != 4)
    DUNE_THROW(MathError, "Power implementation does not compute the correct result");

  if (Power<2>::eval(4) != 16)
    DUNE_THROW(MathError, "Power implementation does not compute the correct result");

  if (Power<3>::eval(4) != 64)
    DUNE_THROW(MathError, "Power implementation does not compute the correct result");

  return 0;
}
catch (Exception& e)
{
  std::cout << e.what() << std::endl;
}
