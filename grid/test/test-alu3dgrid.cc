// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <iostream>
#include <sstream>
#include <string>

#include <dune/grid/alu3dgrid.hh>

#include "gridcheck.cc"
#include "checkgeometryinfather.cc"
#include "checkintersectionit.cc"

using namespace Dune;

int main () {
  try {
    /* use grid-file appropriate for dimensions */
    std::string filename_hexa("alu-testgrid.hexa");
    std::string filename_tetra("alu-testgrid.tetra");

    // extra-environment to check destruction
    {
      factorEpsilon = 500.0;
      // be careful, each global refine create 8 x maxlevel elements
      int mxl = 2;
      {
        ALU3dGrid<3,3,hexa>
        grid_hexa(filename_hexa);

        gridcheck(grid_hexa);
        for(int i=0; i<mxl; i++) {
          grid_hexa.globalRefine(1);
          gridcheck(grid_hexa);
        }

        // check the method geometryInFather()
        checkGeometryInFather(grid_hexa);

        // check the intersection iterator and the geometries it returns
        checkIntersectionIterator(grid_hexa);
      }
      {
        ALU3dGrid<3,3,tetra>
        grid_tetra(filename_tetra);

        gridcheck(grid_tetra);
        for(int i=0; i<mxl; i++) {
          grid_tetra.globalRefine(1);
          gridcheck(grid_tetra);
        }

        // check the method geometryInFather()
        checkGeometryInFather(grid_tetra);

        // check the intersection iterator and the geometries it returns
        checkIntersectionIterator(grid_hexa);
      }
    };

  } catch (Dune::Exception &e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }

  return 0;
}
