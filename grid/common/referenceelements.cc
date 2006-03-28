// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include "config.h"
#include "referenceelements.hh"

namespace Dune {

  template<typename ctype, int dim>
  ReferenceCubeContainer<ctype,dim> ReferenceElements<ctype,dim>::cube;
  template<typename ctype, int dim>
  ReferenceSimplexContainer<ctype,dim> ReferenceElements<ctype,dim>::simplices;
  template<typename ctype, int dim>
  ReferenceElementContainer<ctype,dim> ReferenceElements<ctype,dim>::general;

  template<typename ctype>
  ReferenceCubeContainer<ctype,3> ReferenceElements<ctype,3>::cube;
  template<typename ctype>
  ReferenceSimplexContainer<ctype,3> ReferenceElements<ctype,3>::simplices;
  template<typename ctype>
  ReferenceElementContainer<ctype,3> ReferenceElements<ctype,3>::general;
  template<typename ctype>
  ReferencePrismContainer<ctype,3> ReferenceElements<ctype,3>::prism;
  template<typename ctype>
  ReferencePyramidContainer<ctype,3> ReferenceElements<ctype,3>::pyramid;

  // we use an empty namespace to initialize the singleton,
  // so that this code is hidden
  namespace {

    template <class C, int d>
    struct InitReferenceElements
    {
      ReferenceCubeContainer<C,d> & a;
      ReferenceSimplexContainer<C,d> & b;
      ReferenceElementContainer<C,d> & c;
      InitReferenceElements() :
        a(ReferenceElements<C,d>::cube),
        b(ReferenceElements<C,d>::simplices),
        c(ReferenceElements<C,d>::general)
      {
        InitReferenceElements<C,d-1> i;
      }
    };

    template <class C>
    struct InitReferenceElements<C,3>
    {
      enum { d=3 };
      ReferenceCubeContainer<C,d> & a;
      ReferenceSimplexContainer<C,d> & b;
      ReferenceElementContainer<C,d> & c;
      ReferencePrismContainer<C,d> & e;
      ReferencePyramidContainer<C,d> & f;
      InitReferenceElements() :
        a(ReferenceElements<C,d>::cube),
        b(ReferenceElements<C,d>::simplices),
        c(ReferenceElements<C,d>::general),
        e(ReferenceElements<C,d>::prism),
        f(ReferenceElements<C,d>::pyramid)
      {
        InitReferenceElements<C,d-1> i;
      }
    };

    template <class C>
    struct InitReferenceElements<C,0>
    {
      ReferenceCubeContainer<C,0> & a;
      ReferenceSimplexContainer<C,0> & b;
      ReferenceElementContainer<C,0> & c;
      enum { d=0 };
      InitReferenceElements() :
        a(ReferenceElements<C,0>::cube),
        b(ReferenceElements<C,0>::simplices),
        c(ReferenceElements<C,0>::general)
      {}
    };

    // force creation of symbols and code ...
    void init_referenceelements()
    {
      InitReferenceElements<double, 5> i1;
      InitReferenceElements<float, 5> i2;
    }

  };

};
