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

  /** \todo Please doc me! */
  template< typename DomainFieldType, typename RangeFieldType, int n, int m >
  class FunctionSpace : public DynamicType {
  public:

    /** \todo Please doc me! */
    typedef DomainFieldType DomainField;
    /** \todo Please doc me! */
    typedef RangeFieldType RangeField;

    /** \todo Please doc me! */
    typedef Mat < n, m, RangeField> JacobianRange;
    /** \todo Please doc me! */
    typedef Vec < m,  Mat< n, n, RangeField> > HessianRange ;

    /** \todo Please doc me! */
    typedef Vec<n, DomainField> Domain;
    /** \todo Please doc me! */
    typedef Vec<m, RangeField> Range;

    /** \todo Please doc me! */
    enum { DimDomain = n, DimRange = m};

    /** \todo Please doc me! */
    FunctionSpace ( int ident ) : DynamicType (ident){} ;

  };

  /** @} end documentation group */

}

#endif
