// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_QUADRATURE_HH__
#define __DUNE_QUADRATURE_HH__

#include <vector>

#include "dynamictype.hh"
#include "basefunctions.hh"

template< class FunctionSpaceType> class BaseFunctionSet;


namespace Dune {

  template< class FunctionSpaceType >
  class Quadrature : public DynamicType {

    typedef typename FunctionSpaceType::Domain DomainType ;
    typedef typename FunctionSpaceType::RangeField RangeFieldType ;

    friend class BaseFunctionSet < FunctionSpaceType >  ;

  public:

    Quadrature ( int ident ) : DynamicType (ident) ;

    int getNumberOfQuadPoints () const ;

    const RangeFieldType&  getQuadratureWeights ( int ) const ;

    const DomainType&  getQuadraturePoints (int ) const ;

  private:
    int numberOfPoints_ ;

    std::vector< RangeFieldType > weights_ ;
    std::vector< DomainType > points_ ;

  };

}

#endif
