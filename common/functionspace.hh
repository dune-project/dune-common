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

  /** \brief An arbitrary function space
      Base class for specific function spaces.
   */
  template< typename DomainFieldType, typename RangeFieldType, int n, int m >
  class FunctionSpace : public DynamicType {
  public:

    /** Intrinsic type used for values in the domain field (usually a double) */
    typedef DomainFieldType DomainField;
    /** Intrinsic type used for values in the range field (usually a double) */
    typedef RangeFieldType RangeField;

    /** \todo Please doc me! */
    typedef Mat < n, m, RangeField> JacobianRange;
    /** \todo Please doc me! */
    typedef FieldVector<Mat< n, n, RangeField>, m> HessianRange ;

    /** Type of domain vector (using type of domain field) */
    typedef FieldVector<DomainField, n> Domain;
    /** Type of range vector (using type of range field) */
    typedef FieldVector<RangeField, m> Range;

    /** Remember the dimensions of the domain and range field */
    enum { DimDomain = n, DimRange = m};

    /** Constructor taking an identifier */
    FunctionSpace ( int ident ) : DynamicType (ident){} ;

  };

  /** @} end documentation group */

}

#endif
