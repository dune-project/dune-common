// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __GAUSSQUADRATUREIMP_HH__
#define __GAUSSQUADRATUREIMP_HH__

namespace Dune {

  // calculates the number of points on on segment, i.e. a line
  template <int order>
  struct PointsOnLine
  {
    // from Peters GaussQuadrature, see Constructor
    enum { points = ( order > 17 ) ? 10 : 1 };
  };

  // specialization for the given order
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

  // calculates m^p
  template <int m, int p>
  struct power_M_P
  {
    // power stores m^p
    enum { power = (m * power_M_P<m,p-1>::power ) };
  };

  // end of recursion via specialization
  template <int m>
  struct power_M_P< m , 0>
  {
    // m^0 = 1
    enum { power = 1 };
  };


  template<class Domain, class RangeField, int dim, int order>
  class GaussQuadrature
  {
    // good old times
    typedef RangeField ct;

  public:
    // number of quadrature points on segment line
    enum { m = PointsOnLine<order>::points };

    // the number of quadrature points is m^dim
    enum { n = power_M_P < m , dim >::power };

    // set up quadrature of given order in d dimensions
    GaussQuadrature ();

    // return number of integration points
    int nip ();

    // return local coordinates of integration point i
    Domain& ip (int i);

    // return weight associated with integration point i
    RangeField w (int i);

  private:
    // Vectors storing the quadrature points and weights
    Vec<dim,ct> local[n];
    RangeField weight[n];
  };

} // end namespace

#include "gaussquadimp.cc"

#endif
