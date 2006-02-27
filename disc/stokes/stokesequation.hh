// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_STOKESEQUATION_HH
#define DUNE_STOKESEQUATION_HH
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


// namespace Dune
// {

//   template <class G, class RT>
//   class StokesEquationParameters {
//     typedef typename G::ctype DT;
//     enum {n=G::dimension, m=1};
//     typedef typename G::Traits::template Codim<0>::Entity Entity;

//   public:

//     enum BC {dirichlet=1,neumann=2};

//     //evaluate source term f

//     //! evaluate source term
//     /*! evaluate source term at given location
//       @param[in]  x    position in global coordinates
//       @param[in]  e    entity of codim 0
//       @param[in]  xi   position in reference element of e
//       \return     value of source term
//     */
//     virtual RT f   (const FieldVector<DT,n>& x, const Entity& e,
//                  const FieldVector<DT,n>& xi) const = 0;

//     //! return type of boundary condition at the given global coordinate
//     /*! return type of boundary condition at the given global coordinate
//       @param[in]  x    position in global coordinates
//       \return     boundary condition type given by enum in this class
//     */
//     virtual BC bctype (const FieldVector<DT,n>& x, const Entity& e,
//                     const FieldVector<DT,n>& xi) const = 0;

//     //! evaluate Dirichlet boundary condition at given position
//     /*! evaluate Dirichlet boundary condition at given position
//       @param[in]  x    position in global coordinates
//       \return     boundary condition value
//     */

//     virtual RT g (const FieldVector<DT,n>& x, const Entity& e,
//                const FieldVector<DT,n>& xi) const = 0;
// //! always defeine virtual destructor in abstract base class
//      virtual ~StokesEquationParameters () {}
//   };

//   }//end of namespace
 #endif
