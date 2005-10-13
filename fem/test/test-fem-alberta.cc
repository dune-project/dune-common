// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

/*

   Instantiate Alberta-Grid and feed it to the generic femcheck()

   Note: Albert needs the defines DUNE_PROBLEM_DIM and DUNE_WORLD_DUNE_PROBLEM_DIM on the
   commandline anyway thus we can use them to select the correct class

 */

#include <iostream>
#include <sstream>

#include <dune/grid/albertagrid.hh>

#include "femcheck.cc"

int main () {
  try {
    /* use grid-file appropriate for dimensions */
    std::ostringstream filename;
    filename << "alberta-testgrid-" << DUNE_PROBLEM_DIM
             << "-" << DUNE_WORLD_DIM << ".al";

    std::cout << std::endl << "AlbertaGrid<" << DUNE_PROBLEM_DIM
              << "," << DUNE_WORLD_DIM
              << "> with grid file: " << filename.str()
              << std::endl << std::endl;

    // extra-environment to check destruction
    {
      Dune::AlbertaGrid<DUNE_PROBLEM_DIM,DUNE_WORLD_DIM>
      grid(filename.str().c_str());
      for(int i=0; i<3; i++)
      {
        grid.globalRefine(1);
        femCheck(grid);
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
