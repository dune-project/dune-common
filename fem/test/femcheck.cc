// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

/*

   Implements a generic grid check

   TODO:
   - check return types

 */

#include "../../common/capabilities.hh"
#include "../../common/helpertemplates.hh"
#include "../../common/exceptions.hh"
#include "../../common/stdstreams.hh"

#include <dune/fem/lagrangebase.hh>
#include <dune/grid/common/gridpart.hh>

#include <limits>

class CheckError : public Dune::Exception {};

template <class GridType>
void femCheck (const GridType &grid)
{
  const int dim = GridType::dimension;

  /*
   * Very simple compile test:  try to instantiate a P1 FE space on the grid
   */
  typedef typename Dune::FunctionSpace < double , double, dim , 1 > FuncSpace;
  typedef typename Dune::LevelGridPart<GridType> GridPartType;
  typedef typename Dune::LagrangeDiscreteFunctionSpace < FuncSpace, GridPartType, 1> FuncSpaceType;

  GridPartType gridPart(grid, 0);
  FuncSpaceType funcSpace(gridPart);

};
