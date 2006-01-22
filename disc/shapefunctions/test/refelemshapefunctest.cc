// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>
#include <dune/disc/shapefunctions/lagrangeshapefunctions.hh>

/** \file
    \brief Test for a segmentation fault caused by the interplay
    between reference elements and shape functions

    On some compilers, e.g., g++-4.0 and some optimization levels :-) the code in this test
    used to produce a segfault.  This was apparently due to initialization
    order subtilities.
 */

using namespace Dune;

int main (int argc, char *argv[])
{
  Dune::NewGeometryType cube2d(Dune::NewGeometryType::cube, 2);
  Dune::NewGeometryType cube1d(Dune::NewGeometryType::cube, 1);

  ReferenceElements<double,2>::general(cube2d).subEntity(0,1,0,2);

  Dune::LagrangeShapeFunctions<double, double, 1>::general(cube1d,1);
}
