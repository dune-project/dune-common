// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "config.h"
#include "lagrangeshapefunctions.hh"

namespace Dune {

  template<typename C, typename T, int d>
  P0CubeShapeFunctionSetContainer<C,T,d> LagrangeShapeFunctions<C,T,d>::p0cube;

  template<typename C, typename T, int d>
  P1CubeShapeFunctionSetContainer<C,T,d> LagrangeShapeFunctions<C,T,d>::p1cube;

  template<typename C, typename T, int d>
  P2CubeShapeFunctionSetContainer<C,T,d> LagrangeShapeFunctions<C,T,d>::p2cube;

  template<typename C, typename T, int d>
  LagrangeShapeFunctionSetContainer<C,T,d> LagrangeShapeFunctions<C,T,d>::general;

  template<typename C, typename T, int d>
  P0SimplexShapeFunctionSetContainer<C,T,d> LagrangeShapeFunctions<C,T,d>::p0simplex;

  template<typename C, typename T, int d>
  P1SimplexShapeFunctionSetContainer<C,T,d> LagrangeShapeFunctions<C,T,d>::p1simplex;

  template<typename C, typename T, int d>
  P2SimplexShapeFunctionSetContainer<C,T,d> LagrangeShapeFunctions<C,T,d>::p2simplex;

  namespace {

    template <class C, class T, int d>
    struct InitLagrangeShapefunctions
    {
      P0CubeShapeFunctionSetContainer<C,T,d> & f1;
      P1CubeShapeFunctionSetContainer<C,T,d> & f2;
      P2CubeShapeFunctionSetContainer<C,T,d> & f3;
      P0SimplexShapeFunctionSetContainer<C,T,d> & f4;
      P1SimplexShapeFunctionSetContainer<C,T,d> & f5;
      P2SimplexShapeFunctionSetContainer<C,T,d> & f6;
      LagrangeShapeFunctionSetContainer<C,T,d> & f7;
      InitLagrangeShapefunctions() :
        f1(LagrangeShapeFunctions<C,T,d>::p0cube),
        f2(LagrangeShapeFunctions<C,T,d>::p1cube),
        f3(LagrangeShapeFunctions<C,T,d>::p2cube),
        f4(LagrangeShapeFunctions<C,T,d>::p0simplex),
        f5(LagrangeShapeFunctions<C,T,d>::p1simplex),
        f6(LagrangeShapeFunctions<C,T,d>::p2simplex),
        f7(LagrangeShapeFunctions<C,T,d>::general)
      {
        InitLagrangeShapefunctions<C,T,d-1> i;
      };
    };

    template <class C, class T>
    struct InitLagrangeShapefunctions<T,C,0>
    {
      enum { d=0 };
      InitLagrangeShapefunctions()
      {};
    };

    // force creation of symbols and code ...
    void init_lagrangeshapefunctions()
    {
      InitLagrangeShapefunctions<double, double, 3> i1;
      InitLagrangeShapefunctions<float, double, 3> i2;
      InitLagrangeShapefunctions<double, float, 3> i3;
      InitLagrangeShapefunctions<float, float, 3> i4;
    }
  }

}
