// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

/*

   Instantiate UG-Grid and feed it to the generic gridcheck()

   Note: UG needs -D_2 or -D_3 defined anyway, use it to select correct
   gridfile and class

   Currently UGGrid can only be initialized via the AmiraMeshReader

 */

#include <iostream>

// transform define to value
#ifdef _2
static const int DIM = 2;
#else
static const int DIM = 3;
#endif

#include <dune/grid/uggrid.hh>
#include <dune/io/file/amirameshreader.hh>

#include "gridcheck.cc"

int main () {
  try {
    /* use grid-file appropriate for dimensions */
    std::ostringstream filename;
    filename << "ug-testgrid-" << DIM << ".am";

    std::cout << std::endl << "UGGrid<" << DIM << "," << DIM
              << "> with grid file: " << filename.str()
              << std::endl << std::endl;

    // extra-environment to check destruction
    {
      typedef Dune::UGGrid<DIM,DIM> Grid;
      Grid grid;
      Dune::AmiraMeshReader<Grid>::read(grid, filename.str());

      gridcheck(grid);
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
