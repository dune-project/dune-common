// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GAUSS_SEIDEL_STEP_HH
#define DUNE_GAUSS_SEIDEL_STEP_HH

#include <dune/solver/common/iterationstep.hh>

namespace Dune {

  //! \todo Please doc me!
  template<class OperatorType, class DiscFuncType>
  class GaussSeidelStep : public IterationStep<OperatorType, DiscFuncType>
  {
  public:

    //! Default constructor.  Doesn't init anything
    GaussSeidelStep() {}

    //! Constructor with a linear problem
    GaussSeidelStep(OperatorType& mat, DiscFuncType& x, DiscFuncType& rhs)
      : IterationStep<OperatorType,DiscFuncType>(mat, x, rhs)
    {}

    //! Perform one iteration
    virtual void iterate();

    //! \todo Please doc me!
    virtual DiscFuncType getSol();

    //! \todo Please doc me!
    double residual(int index) const;

  };

}  // namespace Dune

#include "common/gaussseidelstep.cc"

#endif
