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

    /** \todo Please doc me! */
    typedef DomainFieldType DomainField;
    /** \todo Please doc me! */
    typedef RangeFieldType RangeField;

    /** \todo Please doc me! */
    typedef Mat < n, m, RangeField> JacobianRange;
    /** \todo Please doc me! */
    typedef FieldVector<Mat< n, n, RangeField>, m> HessianRange ;

    /** \todo Please doc me! */
    typedef FieldVector<DomainField, n> Domain;
    /** \todo Please doc me! */
    typedef FieldVector<RangeField, m> Range;

    /** \todo Please doc me! */
    enum { DimDomain = n, DimRange = m};

    /** \todo Please doc me! */
    FunctionSpace ( int ident ) : DynamicType (ident){} ;

  };

  /** @} end documentation group */

}

#endif
