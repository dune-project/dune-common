// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

/*

   Instantiate Alberta-Grid and feed it to the generic gridcheck()

   Note: Albert needs the defines DIM and DIM_OF_WORLD on the
   commandline anyway thus we can use them to select the correct class

 */

#include <iostream>
#include <sstream>

#include <dune/grid/albertagrid.hh>

#include "gridcheck.cc"

int main () {
  try {
    /* use grid-file appropriate for dimensions */
    std::ostringstream filename;
    filename << "alberta-testgrid-" << DIM << "-" << DIM_OF_WORLD << ".al";

    std::cout << std::endl << "AlbertaGrid<" << DIM << "," << DIM_OF_WORLD
              << "> with grid file: " << filename.str()
              << std::endl << std::endl;

    // extra-environment to check destruction
    {

      Dune::AlbertaGrid<DIM,DIM_OF_WORLD> grid(filename.str().c_str());
      for(int i=0; i<3; i++)
      {
        grid.globalRefine(1);
        gridcheck(grid);
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
};
