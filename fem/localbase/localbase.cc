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
  inline Vec<LocalBase<ddef,elType,Const>::dimrange>
  LocalBase<ddef,elType,Const>::phi(Vec<ddef> &x)
  {
    Vec<dimrange> tmp(1.0);
    return tmp;
  }

  template <int ddef, ElementType elType>
  inline LocalBase<ddef,elType,Const>::FUNC*
  LocalBase<ddef,elType,Const>::
  getBaseFunc(int i)
  {
    return &phi;
  }


  //! Spezialization for LagrangeOne (linear Lagrange elements)
  template <int ddef, ElementType elType>
  inline LocalBase<ddef,elType,LagrangeOne>::FUNC*
  LocalBase<ddef,elType,LagrangeOne>::
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

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,LagrangeOne>::dimrange>
  LocalBase<ddef,elType,LagrangeOne>::phi0(Vec<dimdef> &x)
  {
    //std::cout << "phi0 \n";
    // Achtung nur 2d
    double val = 1.0-x(0)-x(1);
    Vec<dimrange> tmp(val);
    return tmp;
  }

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,LagrangeOne>::dimrange>
  LocalBase<ddef,elType,LagrangeOne>::phi1(Vec<dimdef> &x)
  {
    //std::cout << "phi1 \n";
    Vec<dimrange> tmp(x(0));
    return tmp;
  }

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,LagrangeOne>::dimrange>
  LocalBase<ddef,elType,LagrangeOne>::phi2(Vec<dimdef> &x)
  {
    //std::cout << "phi2 \n";
    Vec<dimrange> tmp(x(1));
    return tmp;
  }

  //! Spezialization for LagrangeOne (linear Lagrange elements)
  template <int ddef, ElementType elType>
  inline LocalBase<ddef,elType,DGOne>::FUNC*
  LocalBase<ddef,elType,DGOne>::
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

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,DGOne>::dimrange>
  LocalBase<ddef,elType,DGOne>::phi0(Vec<dimdef> &x)
  {
    //std::cout << "phi0 \n";
    // Achtung nur 2d
    double val = 1.0-x(0)-x(1);
    Vec<dimrange> tmp(val);
    return tmp;
  }

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,DGOne>::dimrange>
  LocalBase<ddef,elType,DGOne>::phi1(Vec<dimdef> &x)
  {
    //std::cout << "phi1 \n";
    Vec<dimrange> tmp(x(0));
    return tmp;
  }

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,DGOne>::dimrange>
  LocalBase<ddef,elType,DGOne>::phi2(Vec<dimdef> &x)
  {
    //std::cout << "phi2 \n";
    Vec<dimrange> tmp(x(1));
    return tmp;
  }

  //*****************************************************************
  //! Spezialization for DGTwo (quadratic DG elements)
  //*****************************************************************
  template <int ddef, ElementType elType>
  inline LocalBase<ddef,elType,DGTwo>::FUNC*
  LocalBase<ddef,elType,DGTwo>::
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

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,DGTwo>::dimrange>
  LocalBase<ddef,elType,DGTwo>::phi0(Vec<dimdef> &x)
  {
    Vec<dimrange> tmp(x(0)*(2.0*x(0) - 1.0));
    return tmp;
  }

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,DGTwo>::dimrange>
  LocalBase<ddef,elType,DGTwo>::phi1(Vec<dimdef> &x)
  {
    Vec<dimrange> tmp(x(1)*(2.0*x(1) - 1.0));
    return tmp;
  }

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,DGTwo>::dimrange>
  LocalBase<ddef,elType,DGTwo>::phi2(Vec<dimdef> &x)
  {
    Vec<dimrange> tmp(x(2)*(2.0*x(2) - 1.0));
    return tmp;
  }

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,DGTwo>::dimrange>
  LocalBase<ddef,elType,DGTwo>::phiEdge0(Vec<dimdef> &x)
  {
    Vec<dimrange> tmp(4.0*x(1)*x(2));
    return tmp;
  }

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,DGTwo>::dimrange>
  LocalBase<ddef,elType,DGTwo>::phiEdge1(Vec<dimdef> &x)
  {
    Vec<dimrange> tmp(4.0*x(0)*x(2));
    return tmp;
  }

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,DGTwo>::dimrange>
  LocalBase<ddef,elType,DGTwo>::phiEdge2(Vec<dimdef> &x)
  {
    Vec<dimrange> tmp(4.0*x(0)*x(1));
    return tmp;
  }
  template <int ddef, ElementType elType>
  inline LocalBase<ddef,elType,LagrangeTwo>::FUNC*
  LocalBase<ddef,elType,LagrangeTwo>::
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
  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,LagrangeTwo>::dimrange>
  LocalBase<ddef,elType,LagrangeTwo>::phi0(Vec<dimdef> &x)
  {
    Vec<dimrange> tmp(x(0)*(2.0*x(0) - 1.0));
    return tmp;
  }

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,LagrangeTwo>::dimrange>
  LocalBase<ddef,elType,LagrangeTwo>::phi1(Vec<dimdef> &x)
  {
    Vec<dimrange> tmp(x(1)*(2.0*x(1) - 1.0));
    return tmp;
  }

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,LagrangeTwo>::dimrange>
  LocalBase<ddef,elType,LagrangeTwo>::phi2(Vec<dimdef> &x)
  {
    Vec<dimrange> tmp(x(2)*(2.0*x(2) - 1.0));
    return tmp;
  }

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,LagrangeTwo>::dimrange>
  LocalBase<ddef,elType,LagrangeTwo>::phiEdge0(Vec<dimdef> &x)
  {
    Vec<dimrange> tmp(4.0*x(1)*x(2));
    return tmp;
  }

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,LagrangeTwo>::dimrange>
  LocalBase<ddef,elType,LagrangeTwo>::phiEdge1(Vec<dimdef> &x)
  {
    Vec<dimrange> tmp(4.0*x(0)*x(2));
    return tmp;
  }

  template<int ddef, ElementType elType>
  inline Vec<LocalBase<ddef,elType,LagrangeTwo>::dimrange>
  LocalBase<ddef,elType,LagrangeTwo>::phiEdge2(Vec<dimdef> &x)
  {
    Vec<dimrange> tmp(4.0*x(0)*x(1));
    return tmp;
  }

  //****************************************************************
  //
  // --LocalBaseFunction
  //
  //****************************************************************
  template <class RefElement, class BaseFuncType, int dimrange, int dimdef>
  inline Vec<dimrange> LocalBaseFunction<RefElement,BaseFuncType,dimrange,dimdef>::
  eval (Vec<dimdef> & localCoords)
  {
    return phi_(localCoords);
  }

  template <class RefElement, class BaseFuncType, int dimrange, int dimdef>
  inline Vec<dimrange> LocalBaseFunction<RefElement,BaseFuncType,dimrange,dimdef>::
  evalFirstDrv (Vec<dimdef> &localCoords)
  {
    Vec<dimrange> tmp (0.0);
    return tmp;
  }

  template <class RefElement, class BaseFuncType, int dimrange, int dimdef>
  inline Vec<dimrange> LocalBaseFunction<RefElement,BaseFuncType,dimrange,dimdef>::
  evalSecondDrv (Vec<dimdef> &localCoords)
  {
    Vec<dimrange> tmp (0.0);
    return tmp;
  }

} // end namespace Dune
