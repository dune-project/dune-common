// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BOUNDARYCONDITIONS_HH
#define DUNE_BOUNDARYCONDITIONS_HH

/**
 * @file
 * @brief  Definition of boundary condition types, extend if necessary
 * @author Peter Bastian
 */
namespace Dune
{
  /** @addtogroup DISC
   *
   * @{
   */
  /**
   * @brief define a class containin boundary condition flags
   *
   */

  //! base class that defines the parameters of a the diffusion equation
  struct BoundaryConditions
  {
    // these values are ordered according to precedence
    // neumann has lowest priority
    enum Flags {neumann=1, process=2, dirichlet=3};
  };

  /** @} */
}
#endif
