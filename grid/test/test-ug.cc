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

template <class GridType >
void markOne ( GridType & grid , int num , int ref )
{
  typedef typename GridType::template Codim<0>::LeafIterator LeafIterator;

  int count = 0;

  LeafIterator endit = grid.template leafend  <0> ();
  for(LeafIterator it = grid.template leafbegin<0> (); it != endit ; ++it )
  {
    if(num == count) grid.mark( ref, it );
    count++;
  }

  grid.preAdapt();
  grid.adapt();
  grid.postAdapt();
}

int main () {
  try {

    // extra-environment to check destruction
    {
      std::cout << std::endl << "UGGrid<2,2> with grid file: ug-testgrid-2.am"
                << std::endl << std::endl;
      Dune::UGGrid<2,2> grid;
      Dune::AmiraMeshReader<Dune::UGGrid<2,2> >::read(grid, "ug-testgrid-2.am");

      // check macro grid
      gridcheck(grid);

      // create hybrid grid
      markOne(grid,0,1) ;
      gridcheck(grid);

      grid.globalRefine(1);
      gridcheck(grid);
    }

    {
      std::cout << std::endl << "UGGrid<3,3> with grid file: ug-testgrid-3.am"
                << std::endl << std::endl;
      Dune::UGGrid<3,3> grid;
      Dune::AmiraMeshReader<Dune::UGGrid<3,3> >::read(grid, "ug-testgrid-3.am");

      // check macro grid
      gridcheck(grid);

      // create hybrid grid
      markOne(grid,0,1) ;
      gridcheck(grid);

      grid.globalRefine(1);
      gridcheck(grid);
    }

  } catch (Dune::Exception &e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }

  return 0;
};
