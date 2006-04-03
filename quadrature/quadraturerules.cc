// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "config.h"
#include "quadraturerules.hh"

namespace Dune {

  /** Singleton holding the Gauss points on the interval */
  GaussPoints GaussPointsSingleton::gp;

  /** Singleton holding the Gauss points on the interval */
  SimplexQuadraturePoints<2> SimplexQuadraturePointsSingleton<2>::sqp;

  /** Singleton holding the SimplexQuadrature points dim==3 */
  SimplexQuadraturePoints<3> SimplexQuadraturePointsSingleton<3>::sqp;

  /** Singleton holding the Prism Quadrature points  */
  PrismQuadraturePoints<3> PrismQuadraturePointsSingleton<3>::prqp;

  /** Singleton holding the Quadrature  points  */
  PyramidQuadraturePoints<3> PyramidQuadraturePointsSingleton<3>::pyqp;

  // singleton holding a quadrature rule container
  template<> QuadratureRuleContainer<float, 1> QuadratureRules<float, 1>::rule(19);
  template<> QuadratureRuleContainer<float, 2> QuadratureRules<float, 2>::rule(19);
  template<> QuadratureRuleContainer<float, 3> QuadratureRules<float, 3>::rule(19);

  template<> QuadratureRuleContainer<double, 1> QuadratureRules<double, 1>::rule(19);
  template<> QuadratureRuleContainer<double, 2> QuadratureRules<double, 2>::rule(19);
  template<> QuadratureRuleContainer<double, 3> QuadratureRules<double, 3>::rule(19);

} // namespace
