// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#include <config.h>

#include <iostream>

#include <dune/grid/onedgrid.hh>

#include "gridcheck.cc"
#include "checkgeometryinfather.cc"

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

int main () try
{

  Dune::SimpleVector<double> coords(6);
  coords[0] = -1;
  coords[1] = -0.4;
  coords[2] = 0.1;
  coords[3] = 0.35;
  coords[4] = 0.38;
  coords[5] = 1;

  // extra-environment to check destruction
  {
    std::cout << std::endl << "OneDGrid<1,1>" << std::endl << std::endl;
    Dune::OneDGrid<1,1> grid(coords);

    // check macro grid
    gridcheck(grid);

    // create hybrid grid
    markOne(grid,0,1) ;
    gridcheck(grid);

    grid.globalRefine(1);
    gridcheck(grid);

    checkGeometryInFather(grid);
  };

  return 0;

}
catch (Dune::Exception &e) {
  std::cerr << e << std::endl;
  return 1;
} catch (...) {
  std::cerr << "Generic exception!" << std::endl;
  return 2;
}
