// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_NORM_HH__
#define __DUNE_NORM_HH__

namespace Dune {

  //! Abstract base for classes computing norms of discrete functions
  template <class DiscFuncType>
  class Norm {

  public:

    //! Compute the norm
    virtual double compute(const DiscFuncType& f) = 0;

  };

}

#endif
