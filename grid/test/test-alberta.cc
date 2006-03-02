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
#include "checkgeometryinfather.cc"
#include "checkintersectionit.cc"

template <class GridType >
void markOne ( GridType & grid , int num , int ref )
{
  typedef typename GridType::template Codim<0> :: LeafIterator LeafIterator;

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
    const int dim      = DUNE_PROBLEM_DIM;
    const int dimworld = DUNE_WORLD_DIM;
    /* use grid-file appropriate for dimensions */
    std::ostringstream filename;
    filename << "alberta-testgrid-" << dim
             << "-" << dimworld << ".al";

    std::cout << std::endl << "AlbertaGrid<" << dim
              << "," << dimworld
              << "> with grid file: " << filename.str()
              << std::endl << std::endl;

    // extra-environment to check destruction
    {
      factorEpsilon = 5e2;
      Dune::AlbertaGrid<dim,dimworld> grid(filename.str());

      gridcheck(grid); // check macro grid
      for(int i=0; i<2; i++)
      {
        grid.globalRefine(1);
        gridcheck(grid);
      }

      for(int i=0; i<2; i++)
      {
        markOne(grid,0,dim);
        gridcheck(grid);
      }

      checkGeometryInFather(grid);
      checkIntersectionIterator(grid);
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
