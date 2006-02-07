// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#include <config.h>

#include <iostream>

#include <dune/grid/sgrid.hh>

#include "gridcheck.cc"
#include "checkgeometryinfather.cc"
#include "checkintersectionit.cc"

int main () {
  try {

    int n[] = { 5, 5, 5, 5 };
    double h[] = { 1.0, 2.0, 3.0, 4.0 };

    // extra-environment to check destruction
    {
      std::cout << std::endl << "SGrid<1,1>" << std::endl << std::endl;
      Dune::SGrid<1,1> g0(n, h);
      gridcheck(g0);

      g0.globalRefine(1);
      checkGeometryInFather(g0);
      checkIntersectionIterator(g0);
    };

    {
      std::cout << std::endl << "SGrid<2,2>" << std::endl << std::endl;
      Dune::SGrid<2,2> g1(n, h);
      gridcheck(g1);

      g1.globalRefine(1);
      checkGeometryInFather(g1);
      checkIntersectionIterator(g1);
    };

    {
      std::cout << std::endl << "SGrid<3,3>" << std::endl << std::endl;
      Dune::SGrid<3,3> g2(n, h);
      gridcheck(g2);

      g2.globalRefine(1);
      checkGeometryInFather(g2);
      checkIntersectionIterator(g2);
    };

    {
      std::cout << std::endl << "SGrid<4,4>" << std::endl << std::endl;
      Dune::SGrid<4,4> g3(n, h);
      gridcheck(g3);

      g3.globalRefine(1);
      checkGeometryInFather(g3);
      checkIntersectionIterator(g3);
    };

  } catch (Dune::Exception &e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }

  return 0;
};
