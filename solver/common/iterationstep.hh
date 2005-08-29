// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ITERATIONSTEP_HH
#define DUNE_ITERATIONSTEP_HH

#include <dune/solver/common/numproc.hh>
#include <vector>

namespace Dune {

  //! Base class for iteration steps being called by a linear solver
  template<class OperatorType, class DiscFuncType>
  class IterationStep : public NumProc
  {
  public:

    //! Default constructor
    IterationStep() {}

    /** \brief Destructor */
    virtual ~IterationStep() {}

    //! Constructor being given linear operator, solution and right hand side
    IterationStep(const OperatorType& mat, DiscFuncType& x, DiscFuncType& rhs) {
      mat_ = &mat;
      x_   = &x;
      rhs_ = &rhs;
    }

    //! Set linear operator, solution and right hand side
    virtual void setProblem(const OperatorType& mat, DiscFuncType& x, DiscFuncType& rhs) {
      x_   = &x;
      rhs_ = &rhs;
      mat_ = &mat;
    }

    //! Do the actual iteration
    virtual void iterate() = 0;

    //! Return solution object
    virtual DiscFuncType getSol() = 0;

    //! Return linear operator
    virtual const OperatorType* getMatrix() {return mat_;}

    /** \brief Checks whether all relevant member variables are set
     * \exception SolverError if the iteration step is not set up properly
     */
    virtual void check() const {
#if 0
      if (!x_)
        DUNE_THROW(SolverError, "Iteration step has no solution vector");
      if (!rhs_)
        DUNE_THROW(SolverError, "Iteration step has no right hand side");
      if (!mat_)
        DUNE_THROW(SolverError, "Iteration step has no matrix");
#endif
    }

    //! The solution container
    DiscFuncType* x_;

    //! The container for the right hand side
    DiscFuncType* rhs_;

    //! The linear operator
    const OperatorType* mat_;

    /** \brief A flag for each degree of freedom stating whether the
     * dof is dirichlet or not */
    const std::vector<bool>* dirichletNodes_;

  };

}

#endif
