// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GAUSSQUADRATURE_HH
#define DUNE_GAUSSQUADRATURE_HH

namespace Dune {

  /** \brief Gauss quadrature rules
   * \ingroup Quadrature
     This class provides Gauss quadrature in d space dimensions
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
    FieldVector<ct, dim>& ip (int i);

    //! return weight associated with integration point i
    double w (int i);

  private:
    int n;
    FieldVector<ct, dim> *local;
    double *weight;

    int power (int y, int d);
  };

} // end namespace

#include "gaussquadrature/gaussquadrature.cc"

#endif
