// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ITERATIVE_SOLVER_HH
#define DUNE_ITERATIVE_SOLVER_HH

#include <dune/solver/common/solver.hh>
#include <dune/solver/common/iterationstep.hh>
#include <dune/fem/norms/norm.hh>

namespace Dune {

  /** \brief A generic iterative solver
   *
   *  This class basically implements a loop that calls
   * an iteration procedure (which is to be supplied be
   * the user).  It also monitors convergence. */
  template <class OperatorType, class DiscFuncType>
  class IterativeSolver : public Solver
  {
  public:
    /** \brief Loop, call the iteration procedure
     * and monitor convergence */
    virtual void solve();

    /** \brief Checks whether all relevant member variables are set
     * \exception SolverError if the iteration step is not set up properly
     */
    virtual void check() const;

    //! The maximum number of iterations
    int numIt;

    //! The iteration step used by the algorithm
    IterationStep<OperatorType,DiscFuncType>* iterationStep;

    //! The norm used to measure convergence
    Norm<DiscFuncType>* errorNorm_;

  };

}

#include "common/iterativesolver.cc"

#endif
