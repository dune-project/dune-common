// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ITERATIONSTEP_HH__
#define __DUNE_ITERATIONSTEP_HH__

#include <dune/solver/common/numproc.hh>

namespace Dune {

  template<class OP_TYPE, class VECTOR_TYPE>
  class IterationStep : public NumProc
  {
  public:

    IterationStep() {}

    IterationStep(OP_TYPE& mat, VECTOR_TYPE& x, VECTOR_TYPE& rhs) {
      mat_ = &mat;
      x_   = &x;
      rhs_ = &rhs;
    }

    virtual void iterate() = 0;

    virtual VECTOR_TYPE getSol() = 0;

    VECTOR_TYPE* x_;

    VECTOR_TYPE* rhs_;

    OP_TYPE* mat_;

  };

}

#endif
