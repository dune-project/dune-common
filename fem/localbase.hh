// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_LOCALBASE_HH
#define DUNE_LOCALBASE_HH

//#include "../common/matvec.hh"
//#include "../grid/common/grid.hh"

namespace Dune
{

  enum BaseType {Const , LagrangeOne, LagrangeTwo, DGOne, DGTwo };

  static const char BaseName [5][20] = {"Const", "LagrangeOne", "LagrangeTwo",
                                        "DGOne", "DGTwo" };


  template <int dimdef, int dimrange, int numDof>
  struct FuncTypes
  {
    typedef Mat<numDof,dimrange> DRVType;

    typedef Vec<dimrange> FUNC (Vec<dimdef> &x);
    typedef DRVType DRV1st (Vec<dimdef> &x);
    typedef DRVType DRV2nd (Vec<dimdef> &x);

  };


  //! default Implementation of Lagrange, i.e. Lagrange 0
  //! ord specifies the degree of the Lagrange base
  //! ElementType specifies the element type for which the current
  //! Lagrange base is

  template <int ddef, ElementType elType, BaseType basetype>
  class LocalBase
  {
  public:
    enum { id = 1 };

    enum { dimdef = ddef };
    enum { order = 0 };
    enum { numDof = 1 };
    enum { dimrange = 1 };
    // order of Lagrange element, 1 = linear Lagrange element

    typedef Vec<dimrange> FUNC (Vec<ddef> &x);
    typedef Mat<dimdef,dimrange> DRV1st (Vec<ddef> &x);
    typedef Mat<dimdef,dimrange> DRV2nd (Vec<ddef> &x);

  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);
    DRV1st* getDrv1st(int i);
    DRV2nd* getDrv2nd(int i);

  private:
    static Vec<dimrange> phi(Vec<ddef> &x);
    static Mat<ddef,dimrange> drv1st(Vec<ddef> &x);
    // drv2nd = drv1st = 0

  }; // end default LocalBase


  //! Specialization for Const base functions
  template <int ddef, ElementType elType>
  class LocalBase<ddef,elType,Const>
  {
  public:
    enum { id = 1 };

    enum { dimdef = ddef };
    enum { order = 0 };
    enum { numDof = 1 };
    enum { dimrange = 1 };

    typedef FuncTypes<dimdef,dimrange,numDof> Traits;

    typedef typename Traits::FUNC FUNC;
    typedef typename Traits::DRV1st DRV1st;
    typedef typename Traits::DRV2nd DRV2nd;

    typedef typename Traits::DRVType DRVType;


  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);
    DRV1st* getDrv1st(int i);
    DRV2nd* getDrv2nd(int i);

  private:
    static Vec<dimrange> phi(Vec<ddef> &x);
    static DRVType drv1st(Vec<dimdef> &x);
    // drv2nd = drv1st = 0

  }; // end default LocalBase


  //****************************************************************
  // linear DG elements for triangles (2D)
  //****************************************************************
  template <int ddef>
  class LocalBase<ddef,triangle,DGOne>
  //: public BaseFunction<ddef>
  {
  public:
    enum { dimdef = ddef };
    enum { order = 1 };
    enum { numDof = dimdef+1 };
    enum { dimrange = 1 };

    typedef FuncTypes<dimdef,dimrange,numDof> Traits;

    typedef typename Traits::FUNC FUNC;
    typedef typename Traits::DRV1st DRV1st;
    typedef typename Traits::DRV2nd DRV2nd;

    typedef typename Traits::DRVType DRVType;

  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);
    DRV1st* getDrv1st(int i);
    DRV2nd* getDrv2nd(int i);


  private:
    static Vec<dimrange> phi0(Vec<dimdef> &x); // 1-x-y
    static Vec<dimrange> phi1(Vec<dimdef> &x); // x
    static Vec<dimrange> phi2(Vec<dimdef> &x); // y
    static DRVType drv1st0(Vec<dimdef> &x);
    static DRVType drv1st1(Vec<dimdef> &x);
    static DRVType drv1st2(Vec<dimdef> &x);
    static DRVType drv2nd (Vec<dimdef> &x);
  }; // end linear DG

  //****************************************************************
  // quadratic DG elements for triangles (2D)
  //****************************************************************
  template <int ddef>
  class LocalBase<ddef,triangle,DGTwo>
  //: public BaseFunction<ddef>
  {
  public:
    enum { dimdef = ddef };
    enum { order = 2 };
    enum { numDof = (dimdef+1)*2 };
    enum { dimrange = 1 };
    typedef FuncTypes<dimdef,dimrange,numDof> Traits;

    typedef typename Traits::FUNC FUNC;
    typedef typename Traits::DRV1st DRV1st;
    typedef typename Traits::DRV2nd DRV2nd;

    typedef typename Traits::DRVType DRVType;


  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);
    DRV1st* getDrv1st(int i);
    DRV2nd* getDrv2nd(int i);

  private:
    static Vec<dimrange> phi0(Vec<dimdef> &x);
    static Vec<dimrange> phi1(Vec<dimdef> &x);
    static Vec<dimrange> phi2(Vec<dimdef> &x);
    static Vec<dimrange> phiEdge0(Vec<dimdef> &x);
    static Vec<dimrange> phiEdge1(Vec<dimdef> &x);
    static Vec<dimrange> phiEdge2(Vec<dimdef> &x);
  }; // end quadratic DG


  //***********************************************************
  // linear Lagrange elements for triangles (2D)
  //***********************************************************
  template <int ddef>
  class LocalBase<ddef,triangle,LagrangeOne>
  //: public BaseFunction<ddef>
  {
  public:
    enum { dimdef = ddef };
    enum { order = 1 };
    enum { numDof = dimdef+1 };
    enum { dimrange = 1 };
    // order of Lagrange element, 1 = linear Lagrange element

    typedef FuncTypes<dimdef,dimrange,numDof> Traits;

    typedef typename Traits::FUNC FUNC;
    typedef typename Traits::DRV1st DRV1st;
    typedef typename Traits::DRV2nd DRV2nd;

    typedef typename Traits::DRVType DRVType;

  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);
    DRV1st* getDrv1st(int i);
    DRV2nd* getDrv2nd(int i);

  private:
    static Vec<dimrange> phi0(Vec<dimdef> &x); // 1-x-y
    static Vec<dimrange> phi1(Vec<dimdef> &x); // x
    static Vec<dimrange> phi2(Vec<dimdef> &x); // y
    static DRVType drv1st0(Vec<dimdef> &x);
    static DRVType drv1st1(Vec<dimdef> &x);
    static DRVType drv1st2(Vec<dimdef> &x);
    static DRVType drv2nd (Vec<dimdef> &x);
  }; // end linear Lagrange


  // linear Lagrange elements for triangles (2D)
  template <int ddef>
  class LocalBase<ddef,triangle,LagrangeTwo>
  //: public BaseFunction<ddef>
  {
  public:
    enum { dimdef = ddef };
    enum { order = 2 };
    enum { numDof = (dimdef+1)*2 };
    enum { dimrange = 1 };
    // order of Lagrange element, 1 = linear Lagrange element

    typedef FuncTypes<dimdef,dimrange,numDof> Traits;

    typedef typename Traits::FUNC FUNC;
    typedef typename Traits::DRV1st DRV1st;
    typedef typename Traits::DRV2nd DRV2nd;

    typedef typename Traits::DRVType DRVType;

  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);
    DRV1st* getDrv1st(int i);
    DRV2nd* getDrv2nd(int i);
  private:
    static Vec<dimrange> phi0(Vec<dimdef> &x);
    static Vec<dimrange> phi1(Vec<dimdef> &x);
    static Vec<dimrange> phi2(Vec<dimdef> &x);
    static Vec<dimrange> phiEdge0(Vec<dimdef> &x);
    static Vec<dimrange> phiEdge1(Vec<dimdef> &x);
    static Vec<dimrange> phiEdge2(Vec<dimdef> &x);
  }; // end default LocalBase


  //************************************************************************
  //
  // --LocalBaseFunction
  //
  //************************************************************************
  //! local base functions on the reference element
  template <class RefElement, class BaseFuncType, int dimrange, int dimdef>
  class LocalBaseFunction
  {
    typedef FuncTypes<dimdef,dimrange,dimdef+1> Traits;

    typedef typename Traits::FUNC FUNC;
    typedef typename Traits::DRV1st DRV1st;
    typedef typename Traits::DRV2nd DRV2nd;

    typedef typename Traits::DRVType DRVType;

    FUNC* phi_;
    DRV1st * drv1st_;
    DRV2nd * drv2nd_;

  public:
    LocalBaseFunction(FUNC *p,DRV1st *d1, DRV2nd *d2) :
      phi_(p) , drv1st_(d1), drv2nd_(d2) { };

    //!  evaluation of the base function
    Vec<dimrange> eval (Vec<dimdef> & localCoords);

    //! evaluation of the first derivative of the basefunction
    DRVType evalFirstDrv (Vec<dimdef> &localCoords);

    //! evaluation of the second derivative of the basefunction
    DRVType evalSecondDrv (Vec<dimdef> &localCoords);

  }; // end class LocalBaseFunction

} // end namespace dune


#include "localbase/localbase.cc"

#endif
