// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_LOCALBASE_HH
#define DUNE_LOCALBASE_HH

//#include "../common/matvec.hh"
//#include "../grid/common/grid.hh"

namespace Dune
{

  enum BaseType {Const , LagrangeOne, LagrangeTwo, DGOne, DGTwo , None };

  static const char BaseName [5][20] = {"Const", "LagrangeOne", "LagrangeTwo",
                                        "DGOne", "DGTwo" };


  //************************************************************************
  //  Number Of Dof for an element
  //************************************************************************
  template <class RefElement, BaseType basetype>
  struct NumberOfDof
  {
    enum { numDof = 1};
  };

  template <class RefElement>
  struct NumberOfDof<RefElement,Const>
  {
    enum { numDof = 1};
  };

  template <class RefElement>
  struct NumberOfDof<RefElement,LagrangeOne>
  {
    enum { numDof = RefElement::dofs};
  };

  template <class RefElement>
  struct NumberOfDof<RefElement,LagrangeTwo>
  {
    enum { numDof = 2*RefElement::dofs};
  };

  template <class RefElement>
  struct NumberOfDof<RefElement,DGOne>
  {
    enum { numDof = RefElement::dofs};
  };

  template <class RefElement>
  struct NumberOfDof<RefElement,DGTwo>
  {
    enum { numDof = 2*RefElement::dofs};
  };


  //************************************************************************
  //  Function Types
  //************************************************************************
  template <int dimdef, int dimrange, int numDof, class ct>
  struct FuncTypes
  {
    typedef Mat<dimdef,dimrange,ct> DRVType;

    typedef Vec<dimdef,ct> COORDtype;
    typedef Vec<dimrange,ct> RETtype;

    typedef RETtype FUNC (COORDtype &x);
    typedef DRVType DRV1st (COORDtype &x);
    typedef DRVType DRV2nd (COORDtype &x);

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

    typedef FuncTypes<dimdef,dimrange,numDof,double> Traits;

    typedef typename Traits::FUNC FUNC;
    typedef typename Traits::DRV1st DRV1st;
    typedef typename Traits::DRV2nd DRV2nd;

    typedef typename Traits::COORDtype COORDtype;
    typedef typename Traits::RETtype RETtype;

    typedef typename Traits::DRVType DRVType;


  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);
    DRV1st* getDrv1st(int i);
    DRV2nd* getDrv2nd(int i);

  private:
    static RETtype phi(COORDtype &x);
    static DRVType drv1st(COORDtype &x);
    // drv2nd = drv1st = 0

  }; // end default LocalBase

  //*********************************************************************
  //! Specialization for Const base functions
  //*********************************************************************
  template <int ddef, ElementType elType>
  class LocalBase<ddef,elType,Const>
  {
  public:
    enum { id = 1 };

    enum { dimdef = ddef };
    enum { order = 0 };
    enum { numDof = 1 };
    enum { dimrange = 1 };

    typedef FuncTypes<dimdef,dimrange,numDof,double> Traits;

    typedef typename Traits::FUNC FUNC;
    typedef typename Traits::DRV1st DRV1st;
    typedef typename Traits::DRV2nd DRV2nd;

    typedef typename Traits::COORDtype COORDtype;
    typedef typename Traits::RETtype RETtype;

    typedef typename Traits::DRVType DRVType;


  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);
    DRV1st* getDrv1st(int i);
    DRV2nd* getDrv2nd(int i);

  private:
    static RETtype phi(COORDtype &x);
    static DRVType drv1st(COORDtype &x);
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

    typedef FuncTypes<dimdef,dimrange,numDof,double> Traits;

    typedef typename Traits::FUNC FUNC;
    typedef typename Traits::DRV1st DRV1st;
    typedef typename Traits::DRV2nd DRV2nd;

    typedef typename Traits::COORDtype COORDtype;
    typedef typename Traits::RETtype RETtype;
    typedef typename Traits::DRVType DRVType;

  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);
    DRV1st* getDrv1st(int i);
    DRV2nd* getDrv2nd(int i);


  private:
    static RETtype phi0(COORDtype &x); // 1-x-y
    static RETtype phi1(COORDtype &x); // x
    static RETtype phi2(COORDtype &x); // y
    static DRVType drv1st0(COORDtype &x);
    static DRVType drv1st1(COORDtype &x);
    static DRVType drv1st2(COORDtype &x);
    static DRVType drv2nd (COORDtype &x);
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
    typedef FuncTypes<dimdef,dimrange,numDof,double> Traits;

    typedef typename Traits::FUNC FUNC;
    typedef typename Traits::DRV1st DRV1st;
    typedef typename Traits::DRV2nd DRV2nd;

    typedef typename Traits::COORDtype COORDtype;
    typedef typename Traits::RETtype RETtype;
    typedef typename Traits::DRVType DRVType;


  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);
    DRV1st* getDrv1st(int i);
    DRV2nd* getDrv2nd(int i);

  private:
    static RETtype phi0(COORDtype &x); // 1-x-y
    static RETtype phi1(COORDtype &x); // x
    static RETtype phi2(COORDtype &x); // y
    static RETtype phiEdge0(COORDtype &x);
    static RETtype phiEdge1(COORDtype &x);
    static RETtype phiEdge2(COORDtype &x);
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

    typedef FuncTypes<dimdef,dimrange,numDof,double> Traits;

    typedef typename Traits::FUNC FUNC;
    typedef typename Traits::DRV1st DRV1st;
    typedef typename Traits::DRV2nd DRV2nd;

    typedef typename Traits::COORDtype COORDtype;
    typedef typename Traits::RETtype RETtype;
    typedef typename Traits::DRVType DRVType;

  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);
    DRV1st* getDrv1st(int i);
    DRV2nd* getDrv2nd(int i);

  private:
    static RETtype phi0(COORDtype &x); // 1-x-y
    static RETtype phi1(COORDtype &x); // x
    static RETtype phi2(COORDtype &x); // y
    static DRVType drv1st0(COORDtype &x);
    static DRVType drv1st1(COORDtype &x);
    static DRVType drv1st2(COORDtype &x);
    static DRVType drv2nd (COORDtype &x);
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

    typedef FuncTypes<dimdef,dimrange,numDof,double> Traits;

    typedef typename Traits::FUNC FUNC;
    typedef typename Traits::DRV1st DRV1st;
    typedef typename Traits::DRV2nd DRV2nd;

    typedef typename Traits::COORDtype COORDtype;
    typedef typename Traits::RETtype RETtype;
    typedef typename Traits::DRVType DRVType;

  public:
    LocalBase() {};

    //! FUNC known from BaseFunction
    FUNC* getBaseFunc(int i);
    DRV1st* getDrv1st(int i);
    DRV2nd* getDrv2nd(int i);
  private:
    static RETtype phi0(COORDtype &x); // 1-x-y
    static RETtype phi1(COORDtype &x); // x
    static RETtype phi2(COORDtype &x); // y
    static RETtype phiEdge0(COORDtype &x);
    static RETtype phiEdge1(COORDtype &x);
    static RETtype phiEdge2(COORDtype &x);
  }; // end default LocalBase


  //************************************************************************
  //
  // --LocalBaseFunction
  //
  //************************************************************************
  //! local base functions on the reference element
  template <class RefElement, BaseType basetype, int dimrange, int dimdef>
  class LocalBaseFunction
  {
    //! the number of DOFs is given by RefElement and the type of the base
    enum { numDof = NumberOfDof<RefElement,basetype>::numDof };

    //! find out the needed types for the functions
    typedef FuncTypes<dimdef,dimrange,numDof,double> Traits;

    typedef typename Traits::FUNC FUNC;
    typedef typename Traits::DRV1st DRV1st;
    typedef typename Traits::DRV2nd DRV2nd;

    typedef typename Traits::COORDtype COORDtype;
    typedef typename Traits::RETtype RETtype;
    typedef typename Traits::DRVType DRVType;

    FUNC* phi_;
    DRV1st * drv1st_;
    DRV2nd * drv2nd_;

  public:
    LocalBaseFunction(FUNC *p, DRV1st *d1, DRV2nd *d2) :
      phi_(p) , drv1st_(d1), drv2nd_(d2) { };

    //!  evaluation of the base function
    RETtype eval (COORDtype & localCoords);

    //! evaluation of the first derivative of the basefunction
    DRVType evalFirstDrv (COORDtype &localCoords);

    //! evaluation of the second derivative of the basefunction
    DRVType evalSecondDrv (COORDtype &localCoords);

  }; // end class LocalBaseFunction

} // end namespace dune


#include "localbase/localbase.cc"

#endif
