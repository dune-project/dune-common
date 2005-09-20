// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef __DUNE_GROUNDWATER_HH__
#define __DUNE_GROUNDWATER_HH__

#include <iostream>
#include <iomanip>

#include "common/fvector.hh"
#include "common/fmatrix.hh"
#include "common/exceptions.hh"
#include "grid/common/grid.hh"
#include "grid/common/referenceelements.hh"

/**
 * @file
 * @brief  Base class for defining an instance of the groundwater flow problem
 * @author Peter Bastian
 */
namespace Dune
{
  /** @addtogroup DISC
   *
   * @{
   */
  /**
   * @brief compute local stiffness matrix for conforming finite elements for diffusion equation
   *
   */

  //! base class that defines the parameters of a the diffusion equation
  /*! An interface for defining parameters for the stationary
        diffusion equation

            div j = q; j = -K grad u; in Omega

                u = g on Gamma1;
                j*n = J on Gamma2.

        Template parameters are:

        - Grid  a DUNE grid type
        - RT    type used for return values

   */
  template<class G, class RT>
  class GroundwaterEquationParameters {
    typedef typename G::ctype DT;
    enum {n=G::dimension, m=1};
    typedef typename G::Traits::template Codim<0>::Entity Entity;

  public:
    //! enums for the boundary condition types
    enum BC {neumann=1, process=2, dirichlet=3};

    //! evaluate diffusion tensor
    /*! Evaluate the diffusion tensor at given location
       @param[in]  x    position in global coordinates
       @param[in]  e    entity of codim 0
       @param[in]  xi   position in reference element of e
       @param[out] D    diffusion tensor to be filled
     */
    virtual const FieldMatrix<DT,n,n>& K (const FieldVector<DT,n>& x, const Entity& e,
                                          const FieldVector<DT,n>& xi) const = 0;

    //! evaluate source term
    /*! evaluate source term at given location
       @param[in]  x    position in global coordinates
       @param[in]  e    entity of codim 0
       @param[in]  xi   position in reference element of e
       \return     value of source term
     */
    virtual RT q   (const FieldVector<DT,n>& x, const Entity& e,
                    const FieldVector<DT,n>& xi) const = 0;

    //! return type of boundary condition at the given global coordinate
    /*! return type of boundary condition at the given global coordinate
       @param[in]  x    position in global coordinates
       \return     boundary condition type given by enum in this class
     */
    virtual BC bctype (const FieldVector<DT,n>& x, const Entity& e,
                       const FieldVector<DT,n>& xi) const = 0;

    //! evaluate Dirichlet boundary condition at given position
    /*! evaluate Dirichlet boundary condition at given position
       @param[in]  x    position in global coordinates
       \return     boundary condition value
     */
    virtual RT g (const FieldVector<DT,n>& x, const Entity& e,
                  const FieldVector<DT,n>& xi) const = 0;

    //! evaluate Neumann boundary condition at given position
    /*! evaluate Neumann boundary condition at given position
       @param[in]  x    position in global coordinates
       \return     boundary condition value
     */
    virtual RT J (const FieldVector<DT,n>& x, const Entity& e,
                  const FieldVector<DT,n>& xi) const = 0;


    //! always defeine virtual destructor in abstract base class
    virtual ~GroundwaterEquationParameters () {}
  };

  /** @} */
}
#endif
