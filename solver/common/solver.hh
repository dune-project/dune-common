// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_SOLVER_HH__
#define __DUNE_SOLVER_HH__

#include <dune/solver/common/numproc.hh>

namespace Dune {

  class Solver : public NumProc
  {
  public:

    virtual void preprocess();

    virtual void solve() = 0;

    double tolerance_;

  };

}

void Dune::Solver::preprocess()
{
  // Default: Do nothing
}

#endif
