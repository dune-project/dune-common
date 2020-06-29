// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#include <config.h>

#include <iostream>

#include <dune/common/exceptions.hh>
#include <dune/common/math.hh>
#include <dune/common/power.hh>
#include <dune/common/unused.hh>

using namespace Dune;

int main (int argc, char** argv) try
{
  // Zero and positive powers
  if (power(4,0) != 1)
    DUNE_THROW(MathError, "power(4,0) does not compute the correct result");

  if (power(4,1) != 4)
    DUNE_THROW(MathError, "power(4,1) implementation does not compute the correct result");

  if (power(4,2) != 16)
    DUNE_THROW(MathError, "power(4,2) implementation does not compute the correct result");

  if (power(4,3) != 64)
    DUNE_THROW(MathError, "power(4,3) implementation does not compute the correct result");

  // Negative powers
  if (power(4.0,-1) != 0.25)
    DUNE_THROW(MathError, "power(4,-1) implementation does not compute the correct result");

  if (power(4.0,-2) != 0.0625)
    DUNE_THROW(MathError, "power(4,-2) implementation does not compute the correct result");

  if (power(4.0,-3) != 0.015625)
    DUNE_THROW(MathError, "power(4,-3) implementation does not compute the correct result");

  // Test whether the result can be used in a compile-time expression
  enum { dummy = power(2,2) };

  // Test legacy power implementation
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
