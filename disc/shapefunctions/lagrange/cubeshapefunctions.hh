// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef __DUNE_CUBESHAPEFUNCTIONS_HH__
#define __DUNE_CUBESHAPEFUNCTIONS_HH__

#include <iostream>
#include "common/fvector.hh"
#include "common/exceptions.hh"
#include "common/misc.hh"
#include "grid/common/grid.hh"
#include "grid/common/referenceelements.hh"

/**
 * @file
 * @brief  define Lagrange type shape functions
 * @author Peter Bastian
 */
namespace Dune
{
  /** @addtogroup DISC
   *
   * @{
   */
  /**
   * @brief Shape functions for Lagrange finite elements
   *
   */


  /***********************************************************
  * P0 shape functions for the cube
  ***********************************************************/


  /*!
   * A class for piecewise constant shape functions in a cube.
   * Evaluation is done at the center of the cube
   */
  template<typename C, typename T, int d>
  class P0CubeShapeFunction
  {
  public:

    // compile time sizes
    enum { dim=d };        // maps from R^d
    enum { comps=1 };          // to R^1

    enum { m=1 };     // total number of basis functions

    // export types
    typedef C CoordType;
    typedef T ResultType;
    typedef P0CubeShapeFunction ImplementationType;

    //! make a shape function object
    P0CubeShapeFunction ()
    {
      for (int j=0; j<dim; j++)
        pos[j] = 0.5;
    }

    //! evaluate shape function in local coordinates
    ResultType evaluateFunction (int comp, const FieldVector<CoordType,d>& x) const
    {
      return 1;
    }

    //! evaluate gradient in local coordinates
    ResultType evaluateDerivative (int comp, int dir, const FieldVector<CoordType,d>& x) const
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
    FieldVector<CoordType,d> pos;
  };


  /*! P1CubeShapeFunctionSet implements the interface of
   * LagrangeShapeFunctionSet but is NOT derived from it.
   * S is either P1CubeShapeFunction<..> or LagrangeShapeFunctionWrapper<P1CubeShapeFunction<..> >
   */
  template<typename C, typename T, int d, typename S>
  class P0CubeShapeFunctionSet
  {
  public:

    // compile time sizes
    enum { dim=d };        // maps from R^d
    enum { comps=1 };          // to R^1

    enum { m=1 };     // total number of basis functions

    // export types
    typedef C CoordType;
    typedef T ResultType;
    typedef S value_type;
    typedef typename S::ImplementationType Imp;     // Imp is either S or derived from S

    //! make a shape function object
    P0CubeShapeFunctionSet ()
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
      return cube;
    }

  private:
    S sf;
  };



  //! This are P1 shape functions in the cube without virtual functions
  template<typename C, typename T, int d>
  class P0CubeShapeFunctionSetContainer
  {
  public:
    // compile time sizes
    enum { dim=d };
    enum { comps=1 };
    enum { maxsize=1 };

    // exported types
    typedef C CoordType;
    typedef T ResultType;
    typedef P0CubeShapeFunctionSet<C,T,d,P0CubeShapeFunction<C,T,d> > value_type;

    const value_type& operator() (GeometryType type, int order) const
    {
      if ( (type==cube) || (type==line) || (type==quadrilateral) ||
           (type==hexahedron) )
        return p0cube;
      DUNE_THROW(NotImplemented, "type not implemented yet");
    }
  private:
    value_type p0cube;
  };



  /***********************************************************
  * P1 shape functions for the cube of any dimension
  * The basic classes implement the interface of a
  * LagrangeShapeFunction(Set) but are NOT derived from
  * it. They can be used in simple cases (fixed element type and
  * order) to avoid virtual function calls. The generally usable
  * classes are then wrapped up with the Wrappers to supply
  * the virtual functions of the abstract interface.
  ***********************************************************/

  /*!
   * A class for piecewise bilinear shape functions in the cube.
   * This class implements the interface of ShapeFunction but
   * is NOT derived from it to avoid making the functions virtual at this
   * stage.
   *
   * Let i=(i_{dim-1},...,i_1,i_0) be the binary representation of the shape
   * function number. Then the corresponding shape function can be written as
   *
   *    phi_i (x) = \prod_{j=0}^{dim-1} [ 1-i_j + x_j*(2*i_j-1) ]
   *
   * and its derivative is
   *
   *    d/dx_k phi_i (x) = (2*i_k-1) * \prod_{j!=k} [ 1-i_j + x_j*(2*i_j-1) ]
   *
   * The coefficients a_{ij} = 1-i_j and b_{ij} = 2*i_j-1 are precomputed.
   */
  template<typename C, typename T, int d>
  class P1CubeShapeFunction
  {
  public:

    // compile time sizes
    enum { dim=d };        // maps from R^d
    enum { comps=1 };          // to R^1

    enum { m=1<<dim };     // total number of basis functions

    // export types
    typedef C CoordType;
    typedef T ResultType;
    typedef P1CubeShapeFunction ImplementationType;

    //! make a shape function object
    P1CubeShapeFunction (int i)     // make it the i'th shape function
    {
      number = i;
      //std::cout<<"number="<<number;
      for (int j=0; j<dim; j++)
      {
        a[j] = 1 - ((i>>j)&1);
        b[j] = 2*((i>>j)&1) - 1;
        pos[j] = (i>>j)&1;
      }
    }

    //! must be defaultconstructible
    P1CubeShapeFunction ()
    {}

    //! evaluate shape function in local coordinates
    ResultType evaluateFunction (int comp, const FieldVector<CoordType,d>& x) const
    {
      ResultType phi = a[0]+x[0]*b[0];
      for (int j=1; j<dim; j++) phi *= a[j]+x[j]*b[j];
      return phi;
    }

    //! evaluate gradient in local coordinates
    ResultType evaluateDerivative (int comp, int dir, const FieldVector<CoordType,d>& x) const
    {
      ResultType deriv = b[dir];
      for (int j=0; j<dim; j++)
        if (j!=dir) deriv *= a[j]+x[j]*b[j];
      return deriv;
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
    ResultType a[dim], b[dim];     // store coefficients for this shape function
    FieldVector<CoordType,d> pos;
  };


  /*! P1CubeShapeFunctionSet implements the interface of
   * LagrangeShapeFunctionSet but is NOT derived from it.
   * S is either P1CubeShapeFunction<..> or LagrangeShapeFunctionWrapper<P1CubeShapeFunction<..> >
   */
  template<typename C, typename T, int d, typename S>
  class P1CubeShapeFunctionSet
  {
  public:

    // compile time sizes
    enum { dim=d };        // maps from R^d
    enum { comps=1 };          // to R^1

    enum { m=1<<dim };     // total number of basis functions

    // export types
    typedef C CoordType;
    typedef T ResultType;
    typedef S value_type;
    typedef typename S::ImplementationType Imp;     // Imp is either S or derived from S

    //! make a shape function object
    P1CubeShapeFunctionSet ()
    {
      for (int i=0; i<m; i++)
        sf[i] = Imp(i);         // assignment of derived class objects defined in wrapper
    }

    //! return total number of shape functions
    int size () const
    {
      return m;
    }

    //! total number of shape functions associated with entity in codim
    int size (int entity, int codim) const
    {
      if (codim==dim) return 1;else return 0;
    }

    //! random access to shape functions
    const value_type& operator[] (int i) const
    {
      return sf[i];     // ok derived class reference goes for base class reference
    }

    //! return order
    int order () const
    {
      return 1;
    }

    //! return type of element
    GeometryType type () const
    {
      return cube;
    }

  private:
    S sf[m];
  };



  //! This are P1 shape functions in the cube without virtual functions
  template<typename C, typename T, int d>
  class P1CubeShapeFunctionSetContainer
  {
  public:
    // compile time sizes
    enum { dim=d };
    enum { comps=1 };
    enum { maxsize=1<<dim };

    // exported types
    typedef C CoordType;
    typedef T ResultType;
    typedef P1CubeShapeFunctionSet<C,T,d,P1CubeShapeFunction<C,T,d> > value_type;

    const value_type& operator() (GeometryType type, int order) const
    {
      if ( (type==cube) || (type==line) || (type==quadrilateral) ||
           (type==hexahedron) )
        return p1cube;
      DUNE_THROW(NotImplemented, "type not implemented yet");
    }
  private:
    value_type p1cube;
  };




  /***********************************************************
  * P2 shape functions for the cube of any dimension
  ***********************************************************/

  /*!
   * A class for piecewise quadratic shape functions in the cube.
   *
   * Shape function no. i is given by
   *
   *    phi_i = \prod_{j=0}^{dim-1} { a[j_i] + b[j_i]x_{j_i} + c[j_i]x_{j_i}^2 }
   *
   * where (j_{d-1},...,j_0) is the integer coordinate of the i'th node
   * with j_i \in {0,1,2}.
   */
  template<typename C, typename T, int d>
  class P2CubeShapeFunction
  {
  public:

    // compile time sizes
    enum { dim=d };        // maps from R^d
    enum { comps=1 };          // to R^1

    enum { m=Power_m_p<3,d>::power };     // total number of basis functions

    // export types
    typedef C CoordType;
    typedef T ResultType;
    typedef P2CubeShapeFunction ImplementationType;

    //! make a shape function object
    P2CubeShapeFunction (int no, int en, int co, const FieldVector<int,dim>& ipos)     // make it the i'th shape function
    {
      num = no;
      ent = en;
      cod = co;
      for (int j=0; j<dim; j++)
      {
        if (ipos[j]==0)
        {
          a[j] = 1.0;
          b[j] = -3.0;
          c[j] = 2.0;
          pos[j] = 0;
        }
        if (ipos[j]==1)
        {
          a[j] = 0.0;
          b[j] = 4.0;
          c[j] = -4.0;
          pos[j] = 0.5;
        }
        if (ipos[j]==2)
        {
          a[j] = 0.0;
          b[j] = -1.0;
          c[j] = 2.0;
          pos[j] = 1.0;
        }
      }
    }

    //! must be defaultconstructible
    P2CubeShapeFunction ()
    {       }

    //! evaluate shape function in local coordinates
    ResultType evaluateFunction (int comp, const FieldVector<CoordType,d>& x) const
    {
      ResultType phi = a[0]+x[0]*b[0] +x[0]*x[0]*c[0];
      for (int j=1; j<dim; j++) phi *= a[j]+x[j]*b[j]+x[j]*x[j]*c[j];
      return phi;
    }

    //! evaluate gradient in local coordinates
    ResultType evaluateDerivative (int comp, int dir, const FieldVector<CoordType,d>& x) const
    {
      ResultType deriv = b[dir]+2*c[dir]*x[dir];
      for (int j=0; j<dim; j++)
        if (j!=dir) deriv *= a[j]+x[j]*b[j]+x[j]*x[j]*c[j];
      return deriv;
    }

    //! consecutive number of associated dof within element
    int localindex (int comp) const
    {
      return num;
    }

    //! codim of associated dof
    int codim () const
    {
      return cod;
    }

    //! entity (of codim) of associated dof
    int entity () const
    {
      return ent;
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
    int num;
    int ent;
    int cod;
    ResultType a[dim], b[dim], c[dim];     // store coefficients for this shape function
    FieldVector<CoordType,d> pos;
  };


  /*! P2CubeShapeFunctionSet implements the interface of
   * LagrangeShapeFunctionSet but is NOT derived from it.
   * S is either P1CubeShapeFunction<..> or LagrangeShapeFunctionWrapper<P1CubeShapeFunction<..> >
   */
  template<typename C, typename T, int d, typename S>
  class P2CubeShapeFunctionSet
  {
  public:

    // compile time sizes
    enum { dim=d };        // maps from R^d
    enum { comps=1 };          // to R^1

    enum { m=Power_m_p<3,d>::power };     // total number of basis functions

    // export types
    typedef C CoordType;
    typedef T ResultType;
    typedef S value_type;
    typedef typename S::ImplementationType Imp;     // Imp is either S or derived from S

    //! make a shape function object
    P2CubeShapeFunctionSet ()
    {
      ReferenceCube<C,d> cube;
      int i=0;
      for (int c=0; c<=dim; c++)
        for (int e=0; e<cube.size(c); e++)
        {
          sf[i] = Imp(i,e,c,cube.iposition(e,c));
          i++;
        }
    }

    //! return total number of shape functions
    int size () const
    {
      return m;
    }

    //! total number of shape functions associated with entity in codim
    int size (int entity, int codim) const
    {
      return 1;
    }

    //! random access to shape functions
    const value_type& operator[] (int i) const
    {
      return sf[i];     // ok derived class reference goes for base class reference
    }

    //! return order
    int order () const
    {
      return 2;
    }

    //! return type of element
    GeometryType type () const
    {
      return cube;
    }

  private:
    S sf[m];
  };


  //! This are P1 shape functions in the cube without virtual functions
  template<typename C, typename T, int d>
  class P2CubeShapeFunctionSetContainer
  {
  public:
    // compile time sizes
    enum { dim=d };
    enum { comps=1 };
    enum { maxsize=Power_m_p<3,d>::power };

    // exported types
    typedef C CoordType;
    typedef T ResultType;
    typedef P2CubeShapeFunctionSet<C,T,d,P2CubeShapeFunction<C,T,d> > value_type;

    const value_type& operator() (GeometryType type, int order) const
    {
      if ( (type==cube) || (type==line) || (type==quadrilateral) ||
           (type==hexahedron) )
        return p2cube;
      DUNE_THROW(NotImplemented, "type not implemented yet");
    }
  private:
    value_type p2cube;
  };



  /** @} */
}
#endif
