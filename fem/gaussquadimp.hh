// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __GAUSSQUADRATURE_HH__
#define __GAUSSQUADRATURE_HH__

namespace Dune {

  /** @defgroup Quadrature Quadrature Rules for FE integration

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


  //! calculates the number of points on on segment, i.e. a line
  template <int order>
  struct PointsOnLine
  {
    //! from Peters GaussQuadrature, see Constructor
    enum { points = ( order > 17 ) ? 10 : 1 };
  };

  //! specialization for the given order
  template <> struct PointsOnLine <3>  { enum { points = 2 }; };
  template <> struct PointsOnLine <4>  { enum { points = 3 }; };
  template <> struct PointsOnLine <5>  { enum { points = 3 }; };
  template <> struct PointsOnLine <6>  { enum { points = 4 }; };
  template <> struct PointsOnLine <7>  { enum { points = 4 }; };
  template <> struct PointsOnLine <8>  { enum { points = 5 }; };
  template <> struct PointsOnLine <9>  { enum { points = 5 }; };
  template <> struct PointsOnLine <10> { enum { points = 6 }; };
  template <> struct PointsOnLine <11> { enum { points = 6 }; };
  template <> struct PointsOnLine <12> { enum { points = 7 }; };
  template <> struct PointsOnLine <13> { enum { points = 7 }; };
  template <> struct PointsOnLine <14> { enum { points = 8 }; };
  template <> struct PointsOnLine <15> { enum { points = 8 }; };
  template <> struct PointsOnLine <16> { enum { points = 9 }; };
  template <> struct PointsOnLine <17> { enum { points = 9 }; };
  // other specialization possible

  //! calculates m^dim
  template <int m, int dim>
  struct mPowerDim
  {
    //! power stores m^dim
    enum { power = ( dim < 1 ) ? 1 : (m * mPowerDim<m,dim-1>::power ) };
  };

  //! end of recursion via specialization
  template <int m>
  struct mPowerDim<m,0>
  {
    enum { power = 1 };
  };


  template<class Domain, class RangeField, int dim, int order>
  class GaussQuadrature {
  public:
    // good old times
    typedef RangeField ct;

    //! number of quadrature points on segment line
    enum { m = PointsOnLine<order>::points };

    //! the number of quadrature points is m^dim
    enum { n = mPowerDim<m,dim>::power };

    //! set up quadrature of given order in d dimensions
    GaussQuadrature ();

    //! return number of integration points
    int nip ();

    //! return local coordinates of integration point i
    Domain& ip (int i);

    //! return weight associated with integration point i
    RangeField w (int i);

  private:
    //! Vector storing the quadrature points
    Mat<dim,n,ct> local;
    RangeField weight[n];
  };

  /** @} end documentation group */

} // end namespace

#include "gaussquadimp.cc"

#endif
