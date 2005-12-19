// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>
#include <dune/disc/shapefunctions/lagrangeshapefunctions.hh>

/** \file
    \brief Test for a segmentation fault caused by the interplay
    between reference elements and shape functions

    On some compilers and some optimization levels :-) the code in this test
    used to produce a segfault.  This was apparently due to initialization
    order subtilities.
 */

using namespace Dune;

int main (int argc, char *argv[])
{
  int a=ReferenceElements<double,2>::general(cube).subEntity(0,1,0,2);

  const LagrangeShapeFunctionSetContainer<double, double, 1>& lagrangeShapeFunctionSet =
    Dune::LagrangeShapeFunctions<double, double, 1>::general;
}
