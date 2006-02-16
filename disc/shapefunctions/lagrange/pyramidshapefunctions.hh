// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_PYRAMIDSHAPEFUNCTIONS_HH
#define DUNE_PYRAMIDSHAPEFUNCTIONS_HH

#include <iostream>
#include "common/fvector.hh"
#include "common/exceptions.hh"
#include "grid/common/grid.hh"

/**
 * @file dune/disc/shapefunctions/lagrange/pyramidshapefunctions.hh
 * @brief  define Lagrange type shape functions for pyramidal elements
 * @author Sreejith Pulloor Kuttanikkad
 */

namespace Dune
{

  /** @addtogroup DISC_Shapefnkt
   *
   * @{
   */

  /***********************************************************
  * The interface for  pyramid shape functions of arbitrary
  * order
  ***********************************************************/


  /***********************************************************
  * P0 shape functions for pyramid
  ***********************************************************/

  /*!
   * A class for piecewise constant shape functions in a pyramid
   * Evaluation is done at the centroid(Centre of gravity) of the pyramid
   */
  template<typename C, typename T>
  class P0PyramidShapeFunction
  {
  public:

    // compile time sizes
    enum { dim=3 };        // maps from R^d
    enum { comps=1 };          // to R^1

    enum { m=1 };     // total number of basis functions

    // export types
    typedef C CoordType;
    typedef T ResultType;
    typedef P0PyramidShapeFunction ImplementationType;

    //! make a shape function object
    P0PyramidShapeFunction ()
    {
      pos[0] = 2/5.0;
      pos[1] = 2/5.0;
      pos[2] = 1/5.0;

    }


    //! evaluate shape function in local coordinates
    ResultType evaluateFunction (int comp, const FieldVector<CoordType,3>& x) const
    {
      return 1;
    }

    //! evaluate gradient in local coordinates
    ResultType evaluateDerivative (int comp, int dir, const FieldVector<CoordType,3>& x) const
    {
      return 0;
    }

    //! consecutive number of associated dof within element
    int localindex (int comp) const
    {
      return 0;
    }

    //! codim of associated dof
    int codim () const
    {
      return 0;
    }

    //! entity (of codim) of associated dof
    int entity () const
    {
      return 0;
    }

    //! consecutive number of dof within entity
    int entityindex () const
    {
      return 0;
    }

    //! interpolation point associated with shape function
    const FieldVector<CoordType,dim>& position () const
    {
      return pos;
    }

  private:
    FieldVector<CoordType,3> pos;
  };


  template<typename C, typename T, typename S>
  class P0PyramidShapeFunctionSet
  {
  public:

    // compile time sizes
    enum { dim=3 };        // maps from R^d
    enum { comps=1 };          // to R^1

    enum { m=1 };     // total number of basis functions

    // export types
    typedef C CoordType;
    typedef T ResultType;
    typedef S value_type;
    typedef typename S::ImplementationType Imp;     // Imp is either S or derived from S

    //! make a shape function object
    P0PyramidShapeFunctionSet ()
    {
      sf = Imp();     // assignment of derived class objects defined in wrapper
    }

    //! return total number of shape functions
    int size () const
    {
      return 1;
    }

    //! total number of shape functions associated with entity in codim
    int size (int entity, int codim) const
    {
      if (codim==0) return 1;else return 0;
    }

    //! random access to shape functions
    const value_type& operator[] (int i) const
    {
      return sf;     // ok derived class reference goes for base class reference
    }

    //! return order
    int order () const
    {
      return 0;
    }

    //! return type of element
    GeometryType type () const
    {
      static GeometryType pyramid(GeometryType::pyramid, dim);
      return pyramid;
    }

  private:
    S sf;
  };


  /***********************************************************
  * P1 shape functions for pyramid
  ***********************************************************/

  /*!
   * A class for piecewise constant shape functions in a pyramid
   * Evaluation is done at the quadrature points the pyramid
   */
  template<typename C, typename T>
  class P1PyramidShapeFunction
  {
  public:

    // compile time sizes
    enum { dim=3 };        // maps from R^d
    enum { comps=1 };          // to R^1

    enum { m=5 };     // total number of basis functions

    // export types
    typedef C CoordType;
    typedef T ResultType;
    typedef P1PyramidShapeFunction ImplementationType;

    //! make a shape function object
    P1PyramidShapeFunction (int i)
    {
      number =i;

      switch (i)
      {
      case 0 :
        pos[0]=0.0;
        pos[1]=0.0;
        pos[2]=0.0;
        break;
      case 1 :
        pos[0]=1.0;
        pos[1]=0.0;
        pos[2]=0.0;
        break;
      case 2 :
        pos[0]=1.0;
        pos[1]=1.0;
        pos[2]=0.0;
        break;
      case 3 :
        pos[0]=0.0;
        pos[1]=1.0;
        pos[2]=0.0;
        break;
      case 4 :
        pos[0]=0.0;
        pos[1]=0.0;
        pos[2]=1.0;
        break;
      default :
        DUNE_THROW(RangeError, "wrong no of shape fns in Pyramid?");
        break;
      }

    }

    //! must be defaultconstructible
    P1PyramidShapeFunction ()
    {}

    //! evaluate shape function in local coordinates
    ResultType evaluateFunction (int comp, const FieldVector<CoordType,3>& x) const
    {
      ResultType phi;
      switch(number)
      {
      case 0 :
        if(x[0] > x[1])
        {
          phi=((1-x[0])*(1-x[1])-x[2]*(1-x[1]));
          return phi;
        }
        else
        {
          phi=((1-x[0])*(1-x[1])-x[2]*(1-x[0]));
          return phi;
        }
        break;

      case 1 :
        if(x[0] > x[1])
        {
          phi=(x[0]*(1-x[1])-x[2]*x[1]);
          return phi;
        }
        else
        {
          phi=(x[0]*(1-x[1])-x[2]*x[0]);
          return phi;
        }
        break;

      case 2 :
        if(x[0] > x[1])
        {
          phi=(x[0]*x[1]+x[2]*x[1]);
          return phi;
        }
        else
        {
          phi=(x[0]*x[1]+x[2]*x[0]);
          return phi;
        }
        break;

      case 3 :
        if(x[0] > x[1])
        {
          phi=((1-x[0])*x[1]-x[2]*x[1]);
          return phi;
        }
        else
        {
          phi=((1-x[0])*x[1]-x[2]*x[0]);
          return phi;
        }
        break;
      case 4 :

        phi=x[2];
        return phi;

        break;
      default :
        DUNE_THROW(RangeError, "wrong no of shape fns in Pyramid?");
        break;
      }
      return phi;

    }

    //! evaluate gradient in local coordinates
    ResultType evaluateDerivative (int comp, int dir, const FieldVector<CoordType,3>& x) const
    {
      ResultType dphi;
      switch(number)
      {
      case 0 :
        if(x[0] > x[1])
        {
          switch(dir)
          {
          case 0 :
            dphi=-1.0+x[1];
            return dphi;
            break;
          case 1 :
            dphi=-1.0+x[0]+x[2];
            return dphi;
            break;
          case 2 :
            dphi=-1.0+x[1];
            return dphi;
            break;
          }

        }
        else
        {

          switch(dir)
          {
          case 0 :
            dphi=-1.0+x[1]+x[2];
            return dphi;
            break;
          case 1 :
            dphi=-1.0+x[0];
            return dphi;
            break;
          case 2 :
            dphi=-1.0+x[0];
            return dphi;
            break;
          }

        }
        break;

      case 1 :
        if(x[0] > x[1])
        {

          switch(dir)
          {
          case 0 :
            dphi=1.0-x[1];
            return dphi;
            break;
          case 1 :
            dphi=-x[0]-x[2];
            return dphi;
            break;
          case 2 :
            dphi=-x[1];
            return dphi;
            break;
          }
        }
        else
        {

          switch(dir)
          {
          case 0 :
            dphi=1.0-x[1]-x[2];
            return dphi;
            break;
          case 1 :
            dphi=-x[0];
            return dphi;
            break;
          case 2 :
            dphi=-x[0];
            return dphi;
            break;
          }

        }
        break;

      case 2 :
        if(x[0] > x[1])
        {
          switch(dir)
          {
          case 0 :
            dphi=x[1];
            return dphi;
            break;
          case 1 :
            dphi=x[0]+x[2];
            return dphi;
            break;
          case 2 :
            dphi=x[1];
            return dphi;
            break;
          }

        }
        else
        {

          switch(dir)
          {
          case 0 :
            dphi=x[1]+x[2];
            return dphi;
            break;
          case 1 :
            dphi=x[0];
            return dphi;
            break;
          case 2 :
            dphi=x[0];
            return dphi;
            break;
          }

        }
        break;

      case 3 :
        if(x[0] > x[1])
        {

          switch(dir)
          {
          case 0 :
            dphi=-x[1];
            return dphi;
            break;
          case 1 :
            dphi=1.0-x[0]-x[2];
            return dphi;
            break;
          case 2 :
            dphi=-x[1];
            return dphi;
            break;
          }

        }
        else
        {

          switch(dir)
          {
          case 0 :
            dphi=-x[1]-x[2];
            return dphi;
            break;
          case 1 :
            dphi=1.0-x[0];
            return dphi;
            break;
          case 2 :
            dphi=-x[0];
            return dphi;
            break;
          }
        }
        break;
      case 4 :
        switch(dir)
        {
        case 0 :
          dphi=0;
          return dphi;
          break;
        case 1 :
          dphi=0;
          return dphi;
          break;
        case 2 :
          dphi=1.0;
          return dphi;
          break;
        }

        break;
      default :
        DUNE_THROW(RangeError, "wrong no of shape fns in Pyramid?");
        break;
      }
      return 0;
    }

    //! consecutive number of associated dof within element
    int localindex (int comp) const
    {
      return number;
    }

    //! codim of associated dof
    int codim () const
    {
      return dim;
    }

    //! entity (of codim) of associated dof
    int entity () const
    {
      return number;
    }

    //! consecutive number of dof within entity
    int entityindex () const
    {
      return 0;
    }

    //! interpolation point associated with shape function
    const FieldVector<CoordType,dim>& position () const
    {
      return pos;
    }

  private:
    int number;
    FieldVector<CoordType,3> pos;
  };


  template<typename C, typename T, typename S>
  class P1PyramidShapeFunctionSet
  {
  public:

    // compile time sizes
    enum { dim=3 };        // maps from R^d
    enum { comps=1 };          // to R^1

    enum { m=5 };     // total number of basis functions

    // export types
    typedef C CoordType;
    typedef T ResultType;
    typedef S value_type;
    typedef typename S::ImplementationType Imp;     // Imp is either S or derived from S

    //! make a shape function object
    P1PyramidShapeFunctionSet ()
    {
      for (int i=0; i<m; i++)
        sf[i] = Imp(i);           // assignment of derived class objects defined in wrapper
    }

    //! return total number of shape functions
    int size () const
    {
      return m;
    }

    //! total number of shape functions associated with entity in codim
    int size (int entity, int codim) const
    {
      if (codim==dim) return m;else return 0;
    }

    //! random access to shape functions
    const value_type& operator[] (int i) const
    {
      return sf[i];           // ok derived class reference goes for base class reference
    }

    //! return order
    int order () const
    {
      return 1;
    }

    //! return type of element
    GeometryType type () const
    {
      static GeometryType pyramid(GeometryType::pyramid, dim);
      return pyramid;
    }

  private:
    S sf[m];
  };




  //! P0 shape functions in the pyramid without virtual functions
  template<typename C, typename T>
  class P0PyramidShapeFunctionSetContainer
  {
  public:
    // compile time sizes
    enum { dim=3 };
    enum { comps=1 };
    enum { maxsize=1 };

    // exported types
    typedef C CoordType;
    typedef T ResultType;
    typedef P0PyramidShapeFunctionSet<C,T,P0PyramidShapeFunction<C,T> > value_type;

    const value_type& operator() (GeometryType type, int order) const
    {

      if(type.isPyramid()) return p0pyramid;
      DUNE_THROW(NotImplemented, "type not implemented yet");
    }
  private:
    value_type p0pyramid;
  };

  //! P1 shape functions in the pyramid without virtual functions

  template<typename C, typename T>
  class P1PyramidShapeFunctionSetContainer
  {
  public:
    // compile time sizes
    enum { dim=3 };
    enum { comps=1 };
    enum { maxsize=5 };

    // exported types
    typedef C CoordType;
    typedef T ResultType;
    typedef P1PyramidShapeFunctionSet<C,T,P1PyramidShapeFunction<C,T> > value_type;

    const value_type& operator() (GeometryType type, int order) const
    {

      if(type.isPyramid()) return p1pyramid;
      DUNE_THROW(NotImplemented, "type not implemented yet");
    }
  private:
    value_type p1pyramid;
  };
  /** @} */
}
#endif
