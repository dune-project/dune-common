// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_LINEAR_SOLVER_HH__
#define __DUNE_LINEAR_SOLVER_HH__

#include <dune/solver/common/solver.hh>
#include <dune/fem/norms/norm.hh>

namespace Dune {

  /** \brief A generic linear solver
   *
   *  This class basically implements a loop that calls
   * an iteration procedure (which is to be supplied be
   * the user).  It also monitors convergence. */
  template <class OperatorType, class DiscFuncType>
  class LinearSolver : public Solver
  {
  public:
    /** \brief Loop, call the iteration procedure
     * and monitor convergence */
    virtual void solve();

    //! The maximum number of iterations
    int numIt;

    //! The iteration step used by the algorithm
    IterationStep<OperatorType,DiscFuncType>* iterationStep;

    //! The norm used to measure convergence
    Norm<DiscFuncType>* errorNorm_;

  };

}

#include "linearsolver.cc"

#endif
