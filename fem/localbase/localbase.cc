// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
namespace Dune
{
  //****************************************************************
  //
  // --LocalBase
  //
  //****************************************************************

  //! Spezialization for const base functions
  template<int ddef, ElementType elType>
  inline typename LocalBase<ddef,elType,Const>::RETtype
  LocalBase<ddef,elType,Const>::phi(COORDtype &x)
  {
    RETtype tmp(1.0);
    return tmp;
  }
  //! Spezialization for const base functions
  template<int ddef, ElementType elType>
  inline typename LocalBase<ddef,elType,Const>::DRVType
  LocalBase<ddef,elType,Const>::drv1st(COORDtype &x)
  {
    DRVType tmp(0.0);
    return tmp;
  }

  template <int ddef, ElementType elType>
  inline typename LocalBase<ddef,elType,Const>::FUNC*
  LocalBase<ddef,elType,Const>::
  getBaseFunc(int i)
  {
    return &phi;
  }

  template <int ddef, ElementType elType>
  inline typename LocalBase<ddef,elType,Const>::DRV1st*
  LocalBase<ddef,elType,Const>::
  getDrv1st(int i)
  {
    return &drv1st;
  }

  template <int ddef, ElementType elType>
  inline typename LocalBase<ddef,elType,Const>::DRV2nd*
  LocalBase<ddef,elType,Const>::
  getDrv2nd(int i)
  {
    return &drv1st;
  }

  //**************************************************************
  //! Spezialization for LagrangeOne (linear Lagrange elements)
  //**************************************************************
  template <int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeOne>::FUNC*
  LocalBase<ddef,triangle,LagrangeOne>::
  getBaseFunc(int i)
  {
    switch(i)
    {
    case 0 : return &phi0;
    case 1 : return &phi1;
    case 2 : return &phi2;
    }
    std::cout << "Warning: Returning NULL \n";
    return NULL;
  }
  template<int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeOne>::DRV1st*
  LocalBase<ddef,triangle,LagrangeOne>::
  getDrv1st(int i)
  {
    switch(i)
    {
    case 0 : return &drv1st0;
    case 1 : return &drv1st1;
    case 2 : return &drv1st2;
    }
    std::cout << "Warning: Returning NULL \n";
    return NULL;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeOne>::DRV2nd*
  LocalBase<ddef,triangle,LagrangeOne>::
  getDrv2nd(int i)
  {
    switch(i)
    {
    case 0 : return &drv2nd;
    case 1 : return &drv2nd;
    case 2 : return &drv2nd;
    }
    std::cout << "Warning: Returning NULL \n";
    return NULL;
  }



  template<int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeOne>::RETtype
  LocalBase<ddef,triangle,LagrangeOne>::phi0(COORDtype &x)
  {
    //std::cout << "phi1 \n";
    Vec<dimrange> tmp(x(0));
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeOne>::RETtype
  LocalBase<ddef,triangle,LagrangeOne>::phi1(COORDtype &x)
  {
    //std::cout << "phi2 \n";
    Vec<dimrange> tmp(x(1));
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeOne>::RETtype
  LocalBase<ddef,triangle,LagrangeOne>::phi2(COORDtype &x)
  {
    //std::cout << "phi0 \n";
    // Achtung nur 2d
    double val = 1.0-x(0)-x(1);
    Vec<dimrange> tmp(val);
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeOne>::DRVType
  LocalBase<ddef,triangle,LagrangeOne>::drv1st0(COORDtype &x)
  {
    // triangle ==> 3 corners
    DRVType tmp(0.0);
    tmp(0,0) = 1.0;
    return tmp;
  }
  template<int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeOne>::DRVType
  LocalBase<ddef,triangle,LagrangeOne>::drv1st1(COORDtype &x)
  {
    // triangle ==> 3 corners
    DRVType tmp(0.0);
    tmp(1,0) = 1.0;
    return tmp;
  }
  template<int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeOne>::DRVType
  LocalBase<ddef,triangle,LagrangeOne>::drv1st2(COORDtype &x)
  {
    // look in ALBERT Reference manual to see that point 2 of reference
    // element has the coordinates (0,0)

    // triangle ==> 3 corners
    //
    DRVType tmp(-1.0);
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeOne>::DRVType
  LocalBase<ddef,triangle,LagrangeOne>::drv2nd(COORDtype &x)
  {
    DRVType tmp(0.0);
    return tmp;
  }


  //**************************************************************
  //! Spezialization for DGOne (Discontinous Galerkin elements)
  //**************************************************************
  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGOne>::FUNC*
  LocalBase<ddef,triangle,DGOne>::
  getBaseFunc(int i)
  {
    switch(i)
    {
    case 0 : return &phi0;
    case 1 : return &phi1;
    case 2 : return &phi2;
    }
    std::cout << "Warning: Returning NULL \n";
    return NULL;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGOne>::DRV1st*
  LocalBase<ddef,triangle,DGOne>::
  getDrv1st(int i)
  {
    switch(i)
    {
    case 0 : return &drv1st0;
    case 1 : return &drv1st1;
    case 2 : return &drv1st2;
    }
    std::cout << "Warning: Returning NULL \n";
    return NULL;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGOne>::DRV2nd*
  LocalBase<ddef,triangle,DGOne>::
  getDrv2nd(int i)
  {
    switch(i)
    {
    case 0 : return &drv2nd;
    case 1 : return &drv2nd;
    case 2 : return &drv2nd;
    }
    std::cout << "Warning: Returning NULL \n";
    return NULL;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGOne>::RETtype
  LocalBase<ddef,triangle,DGOne>::phi0(COORDtype &x)
  {
    Vec<dimrange> tmp(x(0));
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGOne>::RETtype
  LocalBase<ddef,triangle,DGOne>::phi1(COORDtype &x)
  {
    Vec<dimrange> tmp(x(1));
    return tmp;
  }
  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGOne>::RETtype
  LocalBase<ddef,triangle,DGOne>::phi2(COORDtype &x)
  {
    double val = 1.0-x(0)-x(1);
    Vec<dimrange> tmp(val);
    return tmp;
  }


  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGOne>::DRVType
  LocalBase<ddef,triangle,DGOne>::drv1st0(COORDtype &x)
  {
    // triangle ==> 3 corners
    DRVType tmp(0.0);
    tmp(0,0) = 1.0;
    return tmp;
  }
  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGOne>::DRVType
  LocalBase<ddef,triangle,DGOne>::drv1st1(COORDtype &x)
  {
    // triangle ==> 3 corners
    DRVType tmp(0.0);
    tmp(1,0) = 1.0;
    return tmp;
  }
  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGOne>::DRVType
  LocalBase<ddef,triangle,DGOne>::drv1st2(COORDtype &x)
  {
    // triangle ==> 3 corners
    DRVType tmp(-1.0);
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGOne>::DRVType
  LocalBase<ddef,triangle,DGOne>::drv2nd(COORDtype &x)
  {
    DRVType tmp(0.0);
    return tmp;
  }

  //*****************************************************************
  //! Spezialization for DGTwo (quadratic DG elements)
  //*****************************************************************
  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGTwo>::FUNC*
  LocalBase<ddef,triangle,DGTwo>::
  getBaseFunc(int i)
  {
    switch(i)
    {
    case 0 : return &phi0;
    case 1 : return &phi1;
    case 2 : return &phi2;
    case 3 : return &phiEdge0;
    case 4 : return &phiEdge1;
    case 5 : return &phiEdge2;
    }
    std::cout << "Warning: Returning NULL \n";
    return NULL;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGTwo>::RETtype
  LocalBase<ddef,triangle,DGTwo>::phi0(COORDtype &x)
  {
    Vec<dimrange> tmp(x(0)*(2.0*x(0) - 1.0));
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGTwo>::RETtype
  LocalBase<ddef,triangle,DGTwo>::phi1(COORDtype &x)
  {
    Vec<dimrange> tmp(x(1)*(2.0*x(1) - 1.0));
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGTwo>::RETtype
  LocalBase<ddef,triangle,DGTwo>::phi2(COORDtype &x)
  {
    Vec<dimrange> tmp(x(2)*(2.0*x(2) - 1.0));
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGTwo>::RETtype
  LocalBase<ddef,triangle,DGTwo>::phiEdge0(COORDtype &x)
  {
    Vec<dimrange> tmp(4.0*x(1)*x(2));
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGTwo>::RETtype
  LocalBase<ddef,triangle,DGTwo>::phiEdge1(COORDtype &x)
  {
    Vec<dimrange> tmp(4.0*x(0)*x(2));
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,DGTwo>::RETtype
  LocalBase<ddef,triangle,DGTwo>::phiEdge2(COORDtype &x)
  {
    Vec<dimrange> tmp(4.0*x(0)*x(1));
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeTwo>::FUNC*
  LocalBase<ddef,triangle,LagrangeTwo>::
  getBaseFunc(int i)
  {
    switch(i)
    {
    case 0 : return &phi0;
    case 1 : return &phi1;
    case 2 : return &phi2;
    case 3 : return &phiEdge0;
    case 4 : return &phiEdge1;
    case 5 : return &phiEdge2;
    }
    std::cout << "Warning: Returning NULL \n";
    return NULL;
  }
  template<int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeTwo>::RETtype
  LocalBase<ddef,triangle,LagrangeTwo>::phi0(COORDtype &x)
  {
    Vec<dimrange> tmp(x(0)*(2.0*x(0) - 1.0));
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeTwo>::RETtype
  LocalBase<ddef,triangle,LagrangeTwo>::phi1(COORDtype &x)
  {
    Vec<dimrange> tmp(x(1)*(2.0*x(1) - 1.0));
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeTwo>::RETtype
  LocalBase<ddef,triangle,LagrangeTwo>::phi2(COORDtype &x)
  {
    Vec<dimrange> tmp(x(2)*(2.0*x(2) - 1.0));
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeTwo>::RETtype
  LocalBase<ddef,triangle,LagrangeTwo>::phiEdge0(COORDtype &x)
  {
    Vec<dimrange> tmp(4.0*x(1)*x(2));
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeTwo>::RETtype
  LocalBase<ddef,triangle,LagrangeTwo>::phiEdge1(COORDtype &x)
  {
    Vec<dimrange> tmp(4.0*x(0)*x(2));
    return tmp;
  }

  template<int ddef>
  inline typename LocalBase<ddef,triangle,LagrangeTwo>::RETtype
  LocalBase<ddef,triangle,LagrangeTwo>::phiEdge2(COORDtype &x)
  {
    Vec<dimrange> tmp(4.0*x(0)*x(1));
    return tmp;
  }

  //****************************************************************
  //
  // --LocalBaseFunction
  //
  //****************************************************************
  template <class RefElement, BaseType basetype, int dimrange, int dimdef>
  inline typename LocalBaseFunction<RefElement,basetype,dimrange,dimdef>::RETtype
  LocalBaseFunction<RefElement,basetype,dimrange,dimdef>::
  eval (COORDtype & localCoords)
  {
    return phi_(localCoords);
  }

  template <class RefElement, BaseType basetype, int dimrange, int dimdef>
  inline typename LocalBaseFunction<RefElement,basetype,dimrange,dimdef>::DRVType
  LocalBaseFunction<RefElement,basetype,dimrange,dimdef>::
  evalFirstDrv (COORDtype &localCoords)
  {
    return drv1st_(localCoords);
  }

  template <class RefElement, BaseType basetype, int dimrange, int dimdef>
  inline typename LocalBaseFunction<RefElement,basetype,dimrange,dimdef>::DRVType
  LocalBaseFunction<RefElement,basetype,dimrange,dimdef>::
  evalSecondDrv (COORDtype &localCoords)
  {
    return drv2nd_(localCoords);
  }

} // end namespace Dune
