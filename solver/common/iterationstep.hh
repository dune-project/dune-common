// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ITERATIONSTEP_HH__
#define __DUNE_ITERATIONSTEP_HH__

#include <dune/solver/common/numproc.hh>

namespace Dune {

  //! Base class for iteration steps being called by a linear solver
  template<class OperatorType, class DiscFuncType>
  class IterationStep : public NumProc
  {
  public:

    //! Default constructor
    IterationStep() {}

    //! Constructor being given linear operator, solution and right hand side
    IterationStep(OperatorType& mat, DiscFuncType& x, DiscFuncType& rhs) {
      mat_ = &mat;
      x_   = &x;
      rhs_ = &rhs;
    }

    //! Set linear operator, solution and right hand side
    void setProblem(OperatorType& mat, DiscFuncType& x, DiscFuncType& rhs) {
      mat_ = &mat;
      x_   = &x;
      rhs_ = &rhs;
    }

    //! Do the actual iteration
    virtual void iterate() = 0;

    //! Return solution object
    virtual DiscFuncType getSol() = 0;

    //! Return linear operator
    virtual OperatorType* getMatrix() {return mat_;}

    //! The grid level the iteration step is working on
    virtual int level() const {return level_;}

    DiscFuncType* x_;

    DiscFuncType* rhs_;

    OperatorType* mat_;

    //! The level of a multigrid hierarchy that is iterator is supposed to work on
    int level_;

  };

}

#endif
