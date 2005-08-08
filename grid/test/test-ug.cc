// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#include <config.h>

#include <iostream>

/*

   Instantiate UG-Grid and feed it to the generic gridcheck()

   Currently UGGrid can only be initialized via the AmiraMeshReader

 */

#include <dune/grid/uggrid.hh>
#include <dune/io/file/amirameshreader.hh>

#include "gridcheck.cc"

int main () {
  try {

    // extra-environment to check destruction
    {
      std::cout << std::endl << "UGGrid<2,2> with grid file: ug-testgrid-2.am"
                << std::endl << std::endl;
      Dune::UGGrid<2,2> grid;
      Dune::AmiraMeshReader<Dune::UGGrid<2,2> >::read(grid, "ug-testgrid-2.am");

      gridcheck(grid);
    };

    {
      std::cout << std::endl << "UGGrid<3,3> with grid file: ug-testgrid-3.am"
                << std::endl << std::endl;
      Dune::UGGrid<3,3> grid;
      Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::read(grid, "ug-testgrid-3.am");

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
