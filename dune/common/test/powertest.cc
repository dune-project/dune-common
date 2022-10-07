// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

#include <config.h>

#include <iostream>

#include <dune/common/deprecated.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/math.hh>
#include <dune/common/power.hh>

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
  [[maybe_unused]] constexpr static int dummy = power(2,2);

  // Test legacy power implementation
  DUNE_NO_DEPRECATED_BEGIN
  if (Power<0>::eval(4) != 1)
    DUNE_THROW(MathError, "Power implementation does not compute the correct result");

  if (Power<1>::eval(4) != 4)
    DUNE_THROW(MathError, "Power implementation does not compute the correct result");

  if (Power<2>::eval(4) != 16)
    DUNE_THROW(MathError, "Power implementation does not compute the correct result");

  if (Power<3>::eval(4) != 64)
    DUNE_THROW(MathError, "Power implementation does not compute the correct result");

  DUNE_NO_DEPRECATED_END
  return 0;
}
catch (Exception& e)
{
  std::cout << e.what() << std::endl;
}
