// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __GAUSSQUADRATURE_HH__
#define __GAUSSQUADRATURE_HH__

namespace Dune {

  /** @addtogroup Quadrature

          Integration over polyhedral regions is a fundamental operation in
      the numerical solution of partial differential equations.
          This module provides a number of classes for numerical
          integration on the reference element.

          @{
   */

  /*! This class provides Gauss Quadrature in d space dimensions
      of various orders from 2 to about 20. All rules are tensor
      products of one-dimensional rules.

          Note that Gauss quadrature is restricted to cube elements (i.e.
          line segments, quadrilaterals, hexahedra)

          The dimension is a template parameter (yes, this class template can
          to 5-dimensional integration) as well as the type used for the components
          of the local coordinates used for the Gauss points.
   */
  template<int dim, class ct>
  class GaussQuadrature {
  public:
    //! set up quadrature of given order in d dimensions
    GaussQuadrature (int order);

    //! copy constructor implementing deep copy
    GaussQuadrature (const GaussQuadrature<dim,ct>& gq);

    //! destructor
    ~GaussQuadrature ();

    //! assignment implementing deep copy
    GaussQuadrature<dim,ct>& operator= (const GaussQuadrature<dim,ct>& gq);

    //! return number of integration points
    int nip ();

    //! return local coordinates of integration point i
    Vec<dim,ct>& ip (int i);

    //! return weight associated with integration point i
    double w (int i);

  private:
    int n;
    Vec<dim,ct> *local;
    double *weight;

    int power (int y, int d);
  };

  /** @} end documentation group */

} // end namespace

#include "gaussquadrature/gaussquadrature.cc"

#endif
