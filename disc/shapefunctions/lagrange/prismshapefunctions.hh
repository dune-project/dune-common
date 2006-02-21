// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_PRISMSHAPEFUNCTIONS_HH
#define DUNE_PRISMSHAPEFUNCTIONS_HH

#include <iostream>
#include "common/fvector.hh"
#include "common/exceptions.hh"
#include "common/geometrytype.hh"

/**
 * @file dune/disc/shapefunctions/lagrange/prismshapefunctions.hh
 * @brief  define Lagrange type shape functions for prism elements
 * @author Sreejith Pulloor Kuttanikkad
 */

namespace Dune
{

  /** @addtogroup DISC_Shapefnkt
   *
   * @{
   */


  /***********************************************************
  * P0 shape functions for prism
  ***********************************************************/

  /*!
   * A class for piecewise constant shape functions in a prism
   * Evaluation is done at the centroid(Centre of gravity) of the prism
   */
  template<typename C,typename T>
  class P0PrismShapeFunction
  {
  public:

    // compile time sizes
    enum { dim=3 };        // maps from R^d
    enum { comps=1 };          // to R^1

    enum { m=1 };     // total number of basis functions

    // export types
    typedef C CoordType;
    typedef T ResultType;
    typedef P0PrismShapeFunction ImplementationType;

    //! make a shape function object
    P0PrismShapeFunction ()
    {
      pos[0] = 1/3.0;
      pos[1] = 1/3.0;
      pos[2] = 1/2.0;

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



  /***********************************************************
  * P1 shape functions for prism
  ***********************************************************/

  /*!
   * A class for piecewise linear shape functions in a prism
   * Evaluation is done at the quadrature points of the prism
   */


  /*
        polynomial of the form: ( a[0] + b[0]*x + b[1]*y + b[2]z) * (a[1] + c[0]*x + c[1]*y + c[2]*z)
        derrivative of the form:( aa[0][dir] + bb[0][dir]*x + bb[1][dir]*y + bb[2][dir]*z)
        dir=0 for x, dir=1 for y, dir=2 for z
   */
  template<typename C,typename T>
  class P1PrismShapeFunction
  {
  public:

    enum {dim=3};
    enum {comps=1};
    enum {m=6};     // no of basis functions=3 or 4

    typedef C CoordType;
    typedef T ResultType;
    typedef P1PrismShapeFunction ImplementationType;

    // i'th shape function
    P1PrismShapeFunction(int i)
    {
      number=i;
      switch(i)
      {

      case 0 :
        //--interpolation point associated with shape fn
        pos[0]=0.0;
        pos[1]=0.0;
        pos[2]=0.0;
        //--
        a[0]=1.0;
        a[1]=1.0;
        b[0]=-1.0;
        b[1]=-1.0;
        b[2]=0.0;
        c[0]=0.0;
        c[1]=0.0;
        c[2]=-1.0;
        //x derivative
        aa[0][0]=-1.0;
        bb[0][0]=0.0;
        bb[1][0]=0.0;
        bb[2][0]=1.0;
        //y derivative
        aa[0][1]=-1.0;
        bb[0][1]=0.0;
        bb[1][1]=0.0;
        bb[2][1]=1.0;
        //z derivative
        aa[0][2]=-1.0;
        bb[0][2]=1.0;
        bb[1][2]=1.0;
        bb[2][2]=0.0;
        break;
      case 1 :
        //--interpolation point associated with shape fn
        pos[0]=1.0;
        pos[1]=0.0;
        pos[2]=0.0;
        //--
        a[0]=0.0;
        a[1]=1.0;
        b[0]=1.0;
        b[1]=0.0;
        b[2]=0.0;
        c[0]=0.0;
        c[1]=0.0;
        c[2]=-1.0;
        //x derivative
        aa[0][0]=1.0;
        bb[0][0]=0.0;
        bb[1][0]=0.0;
        bb[2][0]=-1.0;
        //y derivative
        aa[0][1]=0.0;
        bb[0][1]=0.0;
        bb[1][1]=0.0;
        bb[2][1]=0.0;
        //z derivative
        aa[0][2]=0.0;
        bb[0][2]=-1.0;
        bb[1][2]=0.0;
        bb[2][2]=0.0;
        break;
      case 2 :
        //--interpolation point associated with shape fn
        pos[0]=0.0;
        pos[1]=1.0;
        pos[2]=0.0;
        //--
        a[0]=0.0;
        a[1]=1.0;
        b[0]=0.0;
        b[1]=1.0;
        b[2]=0.0;
        c[0]=0.0;
        c[1]=0.0;
        c[2]=-1.0;
        //x derivative
        aa[0][0]=0.0;
        bb[0][0]=0.0;
        bb[1][0]=0.0;
        bb[2][0]=0.0;
        //y derivative
        aa[0][1]=1.0;
        bb[0][1]=0.0;
        bb[1][1]=0.0;
        bb[2][1]=-1.0;
        //z derivative
        aa[0][2]=0.0;
        bb[0][2]=0.0;
        bb[1][2]=-1.0;
        bb[2][2]=0.0;
        break;
      case 3 :
        //--interpolation point associated with shape fn
        pos[0]=0.0;
        pos[1]=0.0;
        pos[2]=1.0;
        //--
        a[0]=1.0;
        a[1]=0.0;
        b[0]=-1.0;
        b[1]=-1.0;
        b[2]=0.0;
        c[0]=0.0;
        c[1]=0.0;
        c[2]=1.0;
        //x derivative
        aa[0][0]=0.0;
        bb[0][0]=0.0;
        bb[1][0]=0.0;
        bb[2][0]=-1.0;
        //y derivative
        aa[0][1]=0.0;
        bb[0][1]=0.0;
        bb[1][1]=0.0;
        bb[2][1]=-1.0;
        //z derivative
        aa[0][2]=1.0;
        bb[0][2]=-1.0;
        bb[1][2]=-1.0;
        bb[2][2]=0.0;
        break;
      case 4 :
        //--interpolation point associated with shape fn
        pos[0]=1.0;
        pos[1]=0.0;
        pos[2]=1.0;
        //--
        a[0]=0.0;
        a[1]=0.0;
        b[0]=1.0;
        b[1]=0.0;
        b[2]=0.0;
        c[0]=0.0;
        c[1]=0.0;
        c[2]=1.0;
        //x derivative
        aa[0][0]=0.0;
        bb[0][0]=0.0;
        bb[1][0]=0.0;
        bb[2][0]=1.0;
        //y derivative
        aa[0][1]=0.0;
        bb[0][1]=0.0;
        bb[1][1]=0.0;
        bb[2][1]=0.0;
        //z derivative
        aa[0][2]=0.0;
        bb[0][2]=1.0;
        bb[1][2]=0.0;
        bb[2][2]=0.0;
        break;
      case 5 :
        //--interpolation point associated with shape fn
        pos[0]=0.0;
        pos[1]=1.0;
        pos[2]=1.0;
        //--
        a[0]=0.0;
        a[1]=0.0;
        b[0]=0.0;
        b[1]=1.0;
        b[2]=0.0;
        c[0]=0.0;
        c[1]=0.0;
        c[2]=1.0;
        //x derivative
        aa[0][0]=0.0;
        bb[0][0]=0.0;
        bb[1][0]=0.0;
        bb[2][0]=0.0;
        //y derivative
        aa[0][1]=0.0;
        bb[0][1]=0.0;
        bb[1][1]=0.0;
        bb[2][1]=1.0;
        //z derivative
        aa[0][2]=0.0;
        bb[0][2]=0.0;
        bb[1][2]=1.0;
        bb[2][2]=0.0;
        break;
      default :
        DUNE_THROW(RangeError, "wrong no of shape fns in Prism?");
        break;
      }
    }
    //! must be defaultconstructible
    P1PrismShapeFunction ()
    {}

    //! evaluate shape function in local coordinates
    ResultType evaluateFunction (int comp, const FieldVector<CoordType,3>& x) const
    {

      ResultType phi1=a[0];
      ResultType phi2=a[1];

      for (int j=0; j<3; ++j)
      {
        phi1 +=b[j]*x[j];
        phi2 +=c[j]*x[j];
      }
      ResultType phi=phi1*phi2;
      return phi;


    }


    ResultType evaluateDerivative (int comp, int dir, const FieldVector<CoordType,3>& x) const

    {
      ResultType deriv=aa[0][dir];

      for (int j=0; j<3; ++j)
      {
        deriv +=bb[j][dir]*x[j];
      }

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
    ResultType a[3], b[3], c[3],aa[3][3], bb[3][3], cc[3][3];
    FieldVector<CoordType,3> pos;


  };



  template<typename C, typename T, typename S>
  class P0PrismShapeFunctionSet
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
    P0PrismShapeFunctionSet ()
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
      static GeometryType prism(GeometryType::prism, dim);
      return prism;
    }

  private:
    S sf;
  };



  template<typename C, typename T, typename S>
  class P1PrismShapeFunctionSet
  {
  public:

    // compile time sizes
    enum { dim=3 };        // maps from R^d
    enum { comps=1 };          // to R^1

    enum { m=6 };     // total number of basis functions

    // export types
    typedef C CoordType;
    typedef T ResultType;
    typedef S value_type;
    typedef typename S::ImplementationType Imp;     // Imp is either S or derived from S

    //! make a shape function object
    P1PrismShapeFunctionSet ()
    {
      for (int i=0; i<m; i++)
        sf[i] = Imp(i);     // assignment of derived class objects defined in wrapper
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
      static GeometryType prism(GeometryType::prism, dim);
      return prism;
    }

  private:
    S sf[m];
  };


  //! P0 shape functions in the prism without virtual functions
  template<typename C, typename T>
  class P0PrismShapeFunctionSetContainer
  {
  public:
    // compile time sizes
    enum { dim=3 };
    enum { comps=1 };
    enum { maxsize=1 };

    // exported types
    typedef C CoordType;
    typedef T ResultType;
    typedef P0PrismShapeFunctionSet<C,T,P0PrismShapeFunction<C,T> > value_type;

    const value_type& operator() (GeometryType type, int order) const
    {
      if(type.isPrism()) return p0prism;
      DUNE_THROW(NotImplemented, "type not implemented yet");
    }

  private:
    value_type p0prism;
  };

  //! P1 shape functions in the prism without virtual functions
  template<typename C, typename T>
  class P1PrismShapeFunctionSetContainer
  {
  public:
    // compile time sizes
    enum { dim=3 };
    enum { comps=1 };
    enum { maxsize=6 };

    // exported types
    typedef C CoordType;
    typedef T ResultType;
    typedef P1PrismShapeFunctionSet<C,T,P1PrismShapeFunction<C,T> > value_type;

    const value_type& operator() (GeometryType type, int order) const
    {
      if(type.isPrism()) return p1prism;
      DUNE_THROW(NotImplemented, "type not implemented yet");
    }

  private:
    value_type p1prism;
  };

  /** @} */

}
#endif
