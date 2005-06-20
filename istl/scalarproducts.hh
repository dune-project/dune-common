// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_SCALARPRODUCTS_HH__
#define __DUNE_SCALARPRODUCTS_HH__

#include <math.h>
#include <complex>
#include <iostream>
#include <iomanip>
#include <string>

#include "solvercategory.hh"

/** \file

   \brief Define base class for scalar product and norm.

   These classes have to be implemented differently for different
   data partitioning strategies. Default implementations for the
   sequential case are provided.

 */

namespace Dune {

  /** @addtogroup ISTL
          @{
   */

  /*! \brief Base class for scalar product and norm computation

      Krylov space methods need to compute scalar products and norms
      (for convergence test only). These methods have to know about the
          underlying data decomposition. For the sequantial case adefault implementation
          is provided.
   */
  template<class X>
  class ScalarProduct {
  public:
    //! export types, they come from the derived class
    typedef X domain_type;
    typedef typename X::field_type field_type;

    /*! \brief Dot product of two vectors.
       It is assumed that the vectors are consistent on the interior+border
       partition.
     */
    virtual field_type dot (const X& x, const X& y) = 0;

    /*! \brief Norm of a right-hand side vector.
       The vector must be consistent on the interior+border partition
     */
    virtual double norm (const X& x) = 0;


    //! every abstract base class has a virtual destructor
    virtual ~ScalarProduct () {}
  };

  //=====================================================================
  // Implementation for ISTL-matrix based operator
  //=====================================================================

  //! Default implementation for the scalar case
  template<class X>
  class SeqScalarProduct : public ScalarProduct<X>
  {
  public:
    //! export types
    typedef X domain_type;
    typedef typename X::field_type field_type;

    //! define the category
    enum {category=SolverCategory::sequential};

    /*! \brief Dot product of two vectors.
       It is assumed that the vectors are consistent on the interior+border
       partition.
     */
    virtual field_type dot (const X& x, const X& y)
    {
      return x*y;
    }

    /*! \brief Norm of a right-hand side vector.
       The vector must be consistent on the interior+border partition
     */
    virtual double norm (const X& x)
    {
      return x.two_norm();
    }
  };



  /** @} end documentation */

} // end namespace

#endif
