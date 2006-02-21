// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BASEFUNCTIONFACTORY_HH
#define DUNE_BASEFUNCTIONFACTORY_HH

#include "basefunctions.hh"

#include <dune/grid/common/grid.hh>

namespace Dune {

  //! \brief Interface class for the generation of base functions.
  //! For every concrete set of base functions, derive your own concrete
  //! base function factory.
  template <class FunctionSpaceImp>
  class BaseFunctionFactory
  {
  public:
    typedef BaseFunctionInterface<FunctionSpaceImp> BaseFunctionType;
  public:
    BaseFunctionFactory(GeometryType geo) :
      geo_(geo)
    {}

    virtual BaseFunctionType* baseFunction(int num) const = 0;

    virtual int numBaseFunctions() const = 0;

    GeometryType geometry() const { return geo_; }
  private:
    GeometryType geo_;
  };


} // end namespace Dune

#endif
