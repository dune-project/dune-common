// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_SOLVER_HH__
#define __DUNE_SOLVER_HH__

#include <dune/solver/common/numproc.hh>

namespace Dune {

  /** \brief The base class for all sorts of solver algorithms */
  class Solver : public NumProc
  {
  public:

    /** \brief Do the necessary preprocessing */
    virtual void preprocess();

    /** \brief Derived classes overload this with the actual
     * solution algorithm */
    virtual void solve() = 0;

    /** \brief The requested tolerance of the solver */
    double tolerance_;

  };

}

void Dune::Solver::preprocess()
{
  // Default: Do nothing
}

#endif
