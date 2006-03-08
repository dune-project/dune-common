// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifdef HAVE_CONFIG_H
# include "config.h"     // autoconf defines, needed by the dune headers
#endif
#include <iostream>
#include <fstream>
#include <iomanip>

#include "dune/grid/common/grid.hh"
#include "dune/grid/sgrid.hh"

#include "dune/grid/common/referenceelements.hh"
#include "dune/quadrature/quadraturerules.hh"


#include "disc/stokes/dgstokes.hh"



/*

   CHANNEL PROBLEM
   Channel- a rectangular region

   Here is a sample problem for the DG Stokes solver:

   The Poiseulle flow between parallel plates (or in channel).
   In this case, if we specify a parabolic inflow, then the same velocity profile
   is maintained through the channel. The pressure drops in a nice linear fashion.

   Classical Poiseuille flow is characterized by a parabolic velocity profile
   over the cross-section of the channel.
   problem here is of finding a velocity vector field u and a pressure scalar field p.



 */




int main()
{
  try
  {
    std::cout<<"Running Channel Problem in 2D):"<<std::endl;

    // create a 2D grid for the given channel
    static const int DIM=2;
    typedef  Dune::SGrid<DIM,DIM> Grid;
    int ELEMENTS[DIM];double CHANNEL[DIM];
    // channel dimension
    CHANNEL[0]=1;        //Length
    CHANNEL[1]=1;        //width
    // discretization
    ELEMENTS[0]=2;        //elements in X dir
    ELEMENTS[1]=2;        // elements in Y dir
    Grid grid(ELEMENTS,CHANNEL);

    // problem with known solution defined in testfunctions.cc
    Example<DIM,double> exact;
    // Parameters for DG Stokes
    DGStokesParameters param;
    // Dirichlet Boundary
    DirichletBoundary<Grid> db(exact);

    RightHandSide<Grid> rh(exact);

    // order of the shape function (velocity)
    static const int ord = 2;

    //change parameters for different DG schemes
    /*
          if sigma > 0 and epsilon = +1 --> NIPG
          if sigma > 0 and epsilon = -1 --> SIPG
          if sigma = 0 and epsilon = +1 --> OBB
     */
    param.epsilon = 1;
    param.sigma = 1.0;
    //viscosity assumed as 1
    param.mu =1.0;

    // std::cout << "epsilon = " << param.epsilon << std::endl;
    //   std::cout << "sigma   = " << param.sigma << std::endl;

    Dune::DGStokes<Grid,ord> dgstokessystem(grid,exact,param,db,rh);

    dgstokessystem.assembleStokesSystem();

    dgstokessystem.solveStokesSystem();

    // stokes system in 2D has 3 variables (u_x, u_y and p)
    double l2error[DIM+1];
    for (int variable=0; variable<=DIM; ++variable)
    {
      l2error[variable]=dgstokessystem.l2errorStokesSystem(variable);
      std::cout<<"L2error for variable " << variable <<": "<<l2error[variable]<<std::endl;
    }


  }
  catch (Dune::Exception &e)
  {
    std::cerr << "Dune reported error: " << e << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception thrown!" << std::endl;
  };
}
