// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_LINEAR_SOLVER_HH__
#define __DUNE_LINEAR_SOLVER_HH__

#include <dune/solver/common/solver.hh>
#include <dune/fem/norms/norm.hh>

namespace Dune {

  template <class OP_TYPE, class VEC_TYPE>
  class LinearSolver : public Solver
  {
  public:
    virtual void solve();


    //protected:

    int numIt;

    IterationStep<OP_TYPE,VEC_TYPE>* iterationStep;

    Norm<VEC_TYPE>* errorNorm_;

  };

}

#include "linearsolver.cc"

#endif
