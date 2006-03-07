// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_STOKESPARAMETERS_HH
#define DUNE_STOKESPARAMETERS_HH
#include <iomanip>

#include "common/fvector.hh"
#include "common/fmatrix.hh"
#include "common/exceptions.hh"
#include "grid/common/grid.hh"
#include "grid/common/referenceelements.hh"


/*
   Stokes Equation

   -(\mu \Delta u)+ \nabla p = f in \Omega

   \nabla \cdot = 0 in \Omega

   u= g on \partial \Omega

 */

class DGStokesParameters
{
public:
  DGStokesParameters() : mu(1.0), sigma(1.0), epsilon(1) {}
  //coeff of viscosity
  double mu;
  // sigma and epsilon are paremetrs for defining the different versions of DG
  /*
       if sigma > 0 and epsilon = +1 --> NIPG
       if sigma > 0 and epsilon = -1 --> SIPG
       if sigma = 0 and epsilon = +1 --> OBB
   */
  double sigma;
  int epsilon;
};



 #endif
