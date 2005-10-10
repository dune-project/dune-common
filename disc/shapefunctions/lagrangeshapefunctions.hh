// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_LAGRANGESHAPEFUNCTIONS_HH
#define DUNE_LAGRANGESHAPEFUNCTIONS_HH

#include <iostream>
#include "common/fvector.hh"
#include "common/exceptions.hh"
#include "common/misc.hh"
#include "grid/common/grid.hh"
#include "grid/common/referenceelements.hh"

#include "shapefunctions.hh"
#include "lagrange/cubeshapefunctions.hh"
#include "lagrange/prismshapefunctions.hh"
#include "lagrange/pyramidshapefunctions.hh"
#include "lagrange/simplexshapefunctions.hh"

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
  * The interface for Lagrange shape functions of arbitrary
  * order and element type.
  ***********************************************************/


  /*! A LagrangeShapeFunction is a scalar (N=1) ShapeFunction extended
      by a method providing a position
   */
  template<typename C, typename T, int d>
  class LagrangeShapeFunction : public ShapeFunction<C,T,d,1>
  {
  public:
    // compile time sizes
    enum { dim=d };
    enum { comps=1 };
    typedef C CoordType;
    typedef T ResultType;

    //! interpolation point associated with shape function
    virtual const FieldVector<CoordType,dim>& position () const = 0;
  };



  /*! A LagrangeShapeFunctionSet is a scalar (N=1) ShapeFunctionSet
   * that returns a LagrangeShapeFunction
   */
  template<typename C, typename T, int d>
  class LagrangeShapeFunctionSet : public ShapeFunctionSet<C,T,d,1>
  {
  public:
    // compile time sizes
    enum { dim=d };
    enum { comps=1 };

    // exported types
    typedef C CoordType;
    typedef T ResultType;
    typedef LagrangeShapeFunction<CoordType,ResultType,dim> value_type;

    //! random access to i'th LagrangeShapeFunction
    virtual const value_type& operator[] (int i) const = 0;
  };


  /***********************************************************
  * Wrappers
  *
  ***********************************************************/

  /*! wrap inlinable implementation into class that is derived
   * from abstract base class and implement the functions with
   * the given implementation
   */
  template<typename Imp>
  class LagrangeShapeFunctionWrapper :
    public LagrangeShapeFunction<typename Imp::CoordType,typename Imp::ResultType,Imp::dim>,
    private Imp
  {
  public:

    // compile time sizes
    enum { dim=Imp::dim };
    enum { comps=1 };

    // exported types
    typedef typename Imp::CoordType CoordType;
    typedef typename Imp::ResultType ResultType;
    typedef Imp ImplementationType;

    //! assignment from implementation type (this class has no data)
    LagrangeShapeFunctionWrapper& operator= (const Imp& imp)
    {
      Imp::operator=(imp);
      return *this;
    }

    //! evaluate component comp at point x
    virtual ResultType evaluateFunction (int comp, const FieldVector<CoordType,dim>& x) const
    {
      return Imp::evaluateFunction(comp,x);
    }

    //! evaluate derivative of component comp in direction dir at point x
    virtual ResultType evaluateDerivative (int comp, int dir, const FieldVector<CoordType,dim>& x) const
    {
      return Imp::evaluateDerivative(comp,dir,x);
    }

    //! consecutive number of associated dof within element
    virtual int localindex (int comp) const
    {
      return Imp::localindex(comp);
    }

    //! codim of associated dof
    virtual int codim () const
    {
      return Imp::codim();
    }

    //! entity (of codim) of associated dof
    virtual int entity () const
    {
      return Imp::entity();
    }

    //! consecutive number of dof within entity
    virtual int entityindex () const
    {
      return Imp::entityindex();
    }

    //! interpolation point associated with shape function
    virtual const FieldVector<CoordType,dim>& position () const
    {
      return Imp::position();
    }
  };




  /*! wrap inlinable implementation into class that is derived
   * from abstract base class and implement the functions with
   * the given implementation
   */
  template<typename Imp>
  class LagrangeShapeFunctionSetWrapper :
    public LagrangeShapeFunctionSet<typename Imp::CoordType,typename Imp::ResultType,Imp::dim>,
    private Imp
  {
  public:

    // compile time sizes
    enum { dim=Imp::dim };
    enum { comps=1 };         // must be available at compile time

    // exported types
    typedef typename Imp::CoordType CoordType;
    typedef typename Imp::ResultType ResultType;
    typedef Imp ImplementationType;
    typedef LagrangeShapeFunction<CoordType,ResultType,dim> value_type;     // Note: Imp::value_type references
    // must be convertible to references to
    // this type !
    //! return total number of shape functions
    virtual int size () const
    {
      return Imp::size();
    }

    //! total number of shape functions associated with entity in codim
    virtual int size (int entity, int codim) const
    {
      return Imp::size(entity,codim);
    }

    //! random access to i'th ShapeFunction
    virtual const value_type& operator[] (int i) const
    {
      return Imp::operator[](i);
    }

    //! return order
    virtual int order () const
    {
      return Imp::order();
    }

    //! return type of element
    virtual GeometryType type () const
    {
      return Imp::type();
    }
  };



  /***********************************************************
  * The general container for Lagrange shape functions of
  * any order and element type (if someone implements them
  * in finite time). All containers are accessible in a singleton.
  ***********************************************************/

  //! This are Lagrange shape functions for any element type and order (in the future ... )
  template<typename C, typename T, int d>
  class LagrangeShapeFunctionSetContainer : public ShapeFunctionSetContainer<C,T,d,1,Power_m_p<2,d>::power >
  {
  public:
    // compile time sizes
    enum { dim=d };
    enum { comps=1 };
    enum { maxsize=Power_m_p<3,dim>::power };

    // exported types
    typedef C CoordType;
    typedef T ResultType;

    //! type of objects in the container
    typedef LagrangeShapeFunctionSet<C,T,d> value_type;

    const value_type& operator() (GeometryType type, int order) const
    {
      if ( (type==cube) || (type==line) || (type==quadrilateral) ||
           (type==hexahedron) )
      {
        if (order==0) return wrappedp0cube;
        if (order==1) return wrappedp1cube;
        if (order==2) return wrappedp2cube;
        DUNE_THROW(RangeError, "order not available for cubes");
      }

      if ( (type==simplex) || (type==triangle) || (type==tetrahedron) )
      {
        if (order==0) return wrappedp0simplex;
        if (order==1) return wrappedp1simplex;
        if (order==2) return wrappedp2simplex;
        DUNE_THROW(RangeError, "order not available for simplex");
      }

      if ( d==3 && type==pyramid)
      {
        if (order==0) return wrappedp0pyramid;
        if (order==1) return wrappedp1pyramid;
        DUNE_THROW(RangeError, "order not available for pyramid");
      }

      if ( d==3 && type==prism)
      {
        if (order==0) return wrappedp0prism;
        if (order==1) return wrappedp1prism;
        else
          DUNE_THROW(RangeError, "order not available for prism");
      }

      DUNE_THROW(RangeError, "type or order not available");
    }

  private:
    // the cubes
    typedef LagrangeShapeFunctionWrapper<P0CubeShapeFunction<C,T,d> > WrappedP0CubeShapeFunction;
    typedef P0CubeShapeFunctionSet<C,T,d,WrappedP0CubeShapeFunction> P0CubeWrappedShapeFunctionSet;
    typedef LagrangeShapeFunctionSetWrapper<P0CubeWrappedShapeFunctionSet> WrappedP0CubeShapeFunctionSet;
    WrappedP0CubeShapeFunctionSet wrappedp0cube;

    typedef LagrangeShapeFunctionWrapper<P1CubeShapeFunction<C,T,d> > WrappedP1CubeShapeFunction;
    typedef P1CubeShapeFunctionSet<C,T,d,WrappedP1CubeShapeFunction> P1CubeWrappedShapeFunctionSet;
    typedef LagrangeShapeFunctionSetWrapper<P1CubeWrappedShapeFunctionSet> WrappedP1CubeShapeFunctionSet;
    WrappedP1CubeShapeFunctionSet wrappedp1cube;

    typedef LagrangeShapeFunctionWrapper<P2CubeShapeFunction<C,T,d> > WrappedP2CubeShapeFunction;
    typedef P2CubeShapeFunctionSet<C,T,d,WrappedP2CubeShapeFunction> P2CubeWrappedShapeFunctionSet;
    typedef LagrangeShapeFunctionSetWrapper<P2CubeWrappedShapeFunctionSet> WrappedP2CubeShapeFunctionSet;
    WrappedP2CubeShapeFunctionSet wrappedp2cube;

    // the simplices
    typedef LagrangeShapeFunctionWrapper<P0SimplexShapeFunction<C,T,d> > WrappedP0SimplexShapeFunction;
    typedef P0SimplexShapeFunctionSet<C,T,d,WrappedP0SimplexShapeFunction> P0SimplexWrappedShapeFunctionSet;
    typedef LagrangeShapeFunctionSetWrapper<P0SimplexWrappedShapeFunctionSet> WrappedP0SimplexShapeFunctionSet;
    WrappedP0SimplexShapeFunctionSet wrappedp0simplex;

    typedef LagrangeShapeFunctionWrapper<P1SimplexShapeFunction<C,T,d> > WrappedP1SimplexShapeFunction;
    typedef P1SimplexShapeFunctionSet<C,T,d,WrappedP1SimplexShapeFunction> P1SimplexWrappedShapeFunctionSet;
    typedef LagrangeShapeFunctionSetWrapper<P1SimplexWrappedShapeFunctionSet> WrappedP1SimplexShapeFunctionSet;
    WrappedP1SimplexShapeFunctionSet wrappedp1simplex;

    typedef LagrangeShapeFunctionWrapper<P2SimplexShapeFunction<C,T,d> > WrappedP2SimplexShapeFunction;
    typedef P2SimplexShapeFunctionSet<C,T,d,WrappedP2SimplexShapeFunction> P2SimplexWrappedShapeFunctionSet;
    typedef LagrangeShapeFunctionSetWrapper<P2SimplexWrappedShapeFunctionSet> WrappedP2SimplexShapeFunctionSet;
    WrappedP2SimplexShapeFunctionSet wrappedp2simplex;

    // Pyramid
    typedef LagrangeShapeFunctionWrapper<P0PyramidShapeFunction<C,T,d> > WrappedP0PyramidShapeFunction;
    typedef P0PyramidShapeFunctionSet<C,T,d,WrappedP0PyramidShapeFunction> P0PyramidWrappedShapeFunctionSet;
    typedef LagrangeShapeFunctionSetWrapper<P0PyramidWrappedShapeFunctionSet> WrappedP0PyramidShapeFunctionSet;
    WrappedP0PyramidShapeFunctionSet wrappedp0pyramid;

    typedef LagrangeShapeFunctionWrapper<P1PyramidShapeFunction<C,T,d> > WrappedP1PyramidShapeFunction;
    typedef P1PyramidShapeFunctionSet<C,T,d,WrappedP1PyramidShapeFunction> P1PyramidWrappedShapeFunctionSet;
    typedef LagrangeShapeFunctionSetWrapper<P1PyramidWrappedShapeFunctionSet> WrappedP1PyramidShapeFunctionSet;
    WrappedP1PyramidShapeFunctionSet wrappedp1pyramid;

    // Prism
    typedef LagrangeShapeFunctionWrapper<P0PrismShapeFunction<C,T,d> > WrappedP0PrismShapeFunction;
    typedef P0PrismShapeFunctionSet<C,T,d,WrappedP0PrismShapeFunction> P0PrismWrappedShapeFunctionSet;
    typedef LagrangeShapeFunctionSetWrapper<P0PrismWrappedShapeFunctionSet> WrappedP0PrismShapeFunctionSet;
    WrappedP0PrismShapeFunctionSet wrappedp0prism;

    typedef LagrangeShapeFunctionWrapper<P1PrismShapeFunction<C,T,d> > WrappedP1PrismShapeFunction;
    typedef P1PrismShapeFunctionSet<C,T,d,WrappedP1PrismShapeFunction> P1PrismWrappedShapeFunctionSet;
    typedef LagrangeShapeFunctionSetWrapper<P1PrismWrappedShapeFunctionSet> WrappedP1PrismShapeFunctionSet;
    WrappedP1PrismShapeFunctionSet wrappedp1prism;
  };


  // singleton holding several reference element containers
  template<typename C, typename T, int d>
  struct LagrangeShapeFunctions {
    static P0CubeShapeFunctionSetContainer<C,T,d> p0cube;
    static P1CubeShapeFunctionSetContainer<C,T,d> p1cube;
    static P2CubeShapeFunctionSetContainer<C,T,d> p2cube;
    static P0SimplexShapeFunctionSetContainer<C,T,d> p0simplex;
    static P1SimplexShapeFunctionSetContainer<C,T,d> p1simplex;
    static P2SimplexShapeFunctionSetContainer<C,T,d> p2simplex;
    static LagrangeShapeFunctionSetContainer<C,T,d> general;
  };

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

  /** @} */
}
#endif
