// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_FUNCTIONSPACE_HH__
#define __DUNE_FUNCTIONSPACE_HH__

#include "matvec.hh"
#include "dynamictype.hh"

namespace Dune {

  /** @defgroup FunctionSpace FunctionSpace
     @ingroup Function
     This provides the interfaces for discrete function spaces.

     @{
   */

  template< typename DomainFieldType, typename RangeFieldType, int n, int m >
  class FunctionSpace : public DynamicType {
  public:

    typedef DomainFieldType DomainField;
    typedef RangeFieldType RangeField;

    typedef Mat < n, m, RangeField> JacobianRange;
    typedef Vec < m,  Mat< n, n, RangeField> > HessianRange ;

    typedef Vec<n, DomainField> Domain;
    typedef Vec<m, RangeField> Range;

    enum { DimDomain = n, DimRange = m};

    FunctionSpace ( int ident ) : DynamicType (ident){} ;

  };

  /** @} end documentation group */

}

#endif
