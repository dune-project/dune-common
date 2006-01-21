// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_FUNCTIONS_HH
#define DUNE_FUNCTIONS_HH

#include <iostream>
#include "common/fvector.hh"
#include "common/exceptions.hh"
#include "grid/common/grid.hh"
#include "grid/utility/hierarchicsearch.hh"

/**
 * @file dune/disc/functions/functions.hh
 * @brief  Hierarchy for functions
 * @author Peter Bastian
 */

/*! @defgroup DISC Abstractions for Discretization Schemes

   @section D1 Introduction ?
   <!--===================-->

   To be written

 */

/*! @defgroup DISC_Functions Functions Hierarchy
   @ingroup DISC
   @brief Hierarchy for functions

   @section D1 Introduction
   <!--=================-->

   To be written
 */

namespace Dune
{
  /** @addtogroup DISC_Functions
   *
   * @{
   */

  //! A constant for infinite differentiability order
  const int InfinitelyDifferentiable=32767;

  //! Abstract base class for functions
  /*! The mother of all functions f : DT^n -> RT^m

        Template parameters are:
     -  DT      type for components of the input vector
     -  RT      type for the return values
     -  n       dimension of the input space
     -  m       number of components of the vector-valued function
   */
  template<class DT, class RT, int n, int m>
  class Function
  {
  public:
    //! export type for domain components
    typedef DT DomainFieldType;

    //! export type for range components
    typedef RT RangeFieldType;

    //! export dimension of domain and range
    enum { DimDomain = n, DimRange = m};

    //! evaluate single component comp at global point x
    /*! Evaluate a single component of the vector-valued
       function.
       @param[in] comp number of component to be evaluated
       @param[in] x    position to be evaluated
       \return         value of the component
     */
    virtual RT eval (int comp, const Dune::FieldVector<DT,n>& x) const = 0;

    //! evaluate all components at point x and store result in y
    /*! Evaluation function for all components at once.
       @param[in]  x    position to be evaluated
       @param[out] y    result vector to be filled
     */
    virtual void evalall (const Dune::FieldVector<DT,n>& x, Dune::FieldVector<RT,m>& y) const = 0;

    virtual ~Function () {}
  };


  //! A class providing default implementations for some method in class Function
  template<class DT, class RT, int n, int m>
  class FunctionDefault : virtual public Function<DT,RT,n,m>
  {
  public:
    //! default implemention for evaluation of all components
    /*! Evaluate all components at once using componentwise evaluation function
       @param[in]  x    position to be evaluated
       @param[out] y    result vector to be filled
     */
    virtual void evalall (const Dune::FieldVector<DT,n>& x, Dune::FieldVector<RT,m>& y) const
    {
      for (int i=0; i<m; ++i)
        y[i] = eval(i,x);
    }
  };





  //! Abstract base class for differentiable functions.
  /*! Extends Function by the evaluation of partial derivatives up to
        a given order.

        Template parameters are:
     -  DT      type for components of the input vector
     -  RT      type for the return values
     -  n       dimension of the input space
     -  m       number of components of the vector-valued function
   */
  template<class DT, class RT, int n, int m>
  class DifferentiableFunction : virtual public Function<DT,RT,n,m>
  {
  public:
    //! evaluate partial derivative
    /*! Evaluate partial derivative of a component of the vector-valued function.
       @param[in]  comp    number of component that should be differentiated
       @param[in]  d       vector giving order of derivative for each variable
       @param[in]  x       position where derivative is to be evaluated
       \return             value of the derivative
     */
    virtual RT derivative (int comp, const Dune::FieldVector<int,n>& d, const Dune::FieldVector<DT,n>& x) const = 0;

    //! return number of partial derivatives that can be taken
    /*! A DifferentiableFunction can say how many derivatives exist
       and can be safely evaluated.
     */
    virtual int order () const = 0;
  };



  //! A class providing default implementations for some method in class Function
  template<class DT, class RT, int n, int m>
  class DifferentiableFunctionDefault : virtual public DifferentiableFunction<DT,RT,n,m>
  {
  public:
    //! default implementation for evaluation of partial derivative
    /*! Evaluate partial derivative of a component of the vector-valued function
       using second order finite differences
       @param[in]  comp    number of component that should be differentiated
       @param[in]  d       vector giving order of derivative for each variable
       @param[in]  x       position where derivative is to be evaluated
       \return             value of the derivative
     */
    RT derivative (int comp, const Dune::FieldVector<int,n>& d, const Dune::FieldVector<DT,n>& x) const
    {
      for (int i=0; i<n; ++i)
        if (d[i]>0)
        {
          DT eps = 1E-5;               // should be type dependent
          if (sizeof(DT)>4) eps = 1E-10;
          DT delta = eps*std::abs(x[i])+eps;
          Dune::FieldVector<int,n> dd(d); dd[i] = dd[i]-1;
          Dune::FieldVector<DT,n> xx(x); xx[i] += delta;
          RT up = derivative(comp,dd,xx);
          xx[i] = x[i]-delta;
          RT down = derivative(comp,dd,xx);
          return (up-down)/(2*delta);
        }
      // d contains all zeros, evaluate function
      return eval(comp,x);
    }
  };



  //! Base class for functions living on a grid
  /*! Grid functions are defined with respect to a grid and allow
        evaluation on elements in local coordinates. Note that derivation
        from the base class Function is public virtual.

        Template parameters are:
     -  G      a grid class, ctype and dimension are used for DT and n in the base class
     -  RT     type for return values
        -  m      number of components of the vector-valued function
   */
  template<class G, class RT, int m>
  class GridFunction : virtual public Function<typename G::ctype,RT,G::dimension,m>
  {
    //! get domain field type from the grid
    typedef typename G::ctype DT;

    //! get domain dimension from the grid
    enum {n=G::dimension};

    //! get entity from the grid
    typedef typename G::Traits::template Codim<0>::Entity Entity;

  public:
    //! evaluate single component comp in the entity e at local coordinates xi
    /*! Evaluate the function in an entity at local coordinates.
       @param[in]  comp   number of component to be evaluated
       @param[in]  e      reference to grid entity of codimension 0
       @param[in]  xi     point in local coordinates of the reference element of e
       \return            value of the component
     */
    virtual RT evallocal (int comp, const Entity& e, const Dune::FieldVector<DT,n>& xi) const = 0;

    //! evaluate all components  in the entity e at local coordinates xi
    /*! Evaluates all components of a function at once.
       @param[in]  e      reference to grid entity of codimension 0
       @param[in]  xi     point in local coordinates of the reference element of e
       @param[out] y      vector with values to be filled
     */
    virtual void evalalllocal (const Entity& e, const Dune::FieldVector<DT,G::dimension>& xi,
                               Dune::FieldVector<RT,m>& y) const = 0;
  };



  //! default implementation for grid function methods
  template<class G, class RT, int m>
  class GridFunctionDefault : virtual public GridFunction<G,RT,m>
  {
    //! get domain field type from the grid
    typedef typename G::ctype DT;

    //! get domain dimension from the grid
    enum {n=G::dimension};

    //! get entity from the grid
    typedef typename G::Traits::template Codim<0>::Entity Entity;

  public:
    //! implement local evaluation with global evaluation
    /*!
       @param[in]  comp   number of component to be evaluated
       @param[in]  e      reference to grid entity of codimension 0
       @param[in]  xi     point in local coordinates of the reference element of e
       \return            value of the component
     */
    RT evallocal (int comp, const Entity& e, const Dune::FieldVector<DT,n>& xi) const
    {
      return eval(comp,e.geometry().global(xi));
    }


    //! does local evaluation via component-wise global evaluation
    /*!
       @param[in]  e      reference to grid entity of codimension 0
       @param[in]  xi     point in local coordinates of the reference element of e
       @param[out] y      vector with values to be filled
     */
    void evalalllocal (const Entity& e, const Dune::FieldVector<DT,G::dimension>& xi,
                       Dune::FieldVector<RT,m>& y) const
    {
      for (int i=0; i<m; ++i)
        y[i] = eval(i,e.geometry().global(xi));
    }

  };


  //! default implementation for global evaluation of a grid function
  template<class G, class RT, class IS, int m>
  class GridFunctionGlobalEvalDefault : virtual public GridFunction<G,RT,m>
  {
    //! get domain field type from the grid
    typedef typename G::ctype DT;

    //! get domain dimension from the grid
    enum {n=G::dimension};

    //! get entity from the grid
    typedef typename G::Traits::template Codim<0>::Entity Entity;

    //! type of EntityPointer
    typedef typename G::Traits::template Codim<0>::EntityPointer EntityPointer;

  public:

    GridFunctionGlobalEvalDefault(const G & _g, const IS& _is) : hsearch(_g,_is) {};

    /*!
       \brief implement global evaluation with local evaluation

       The local entity is searched via a hierarchic search

       @param[in]  comp   number of component to be evaluated
       @param[in]  e      reference to grid entity of codimension 0
       @param[in]  xi     point in local coordinates of the reference element of e
       \return            value of the component
     */
    virtual RT eval (int comp, const Dune::FieldVector<DT,n>& xi) const
    {
      EntityPointer e = hsearch.findEntity(xi);
      std::cout << "Level = " << e->level() << std::endl;
      return evallocal(comp,*e,e->geometry().local(xi));
    }

  private:
    HierarchicSearch<G,IS> hsearch;
  };


  //! Base class for differentiable functions living on a grid
  /*! Functions that are differentiable and live on a grid.

        Template parameters are:
     -  G      a grid class, ctype and dimension are used for DT and n in the base class
     -  RT     type for return values
        -  m      number of components of the vector-valued function
   */
  template<class G, class RT, int m>
  class DifferentiableGridFunction : virtual public GridFunction<G,RT,m>,
                                     virtual public DifferentiableFunction<typename G::ctype,RT,G::dimension,m>
  {
    //! get domain field type from the grid
    typedef typename G::ctype DT;

    //! get domain dimension from the grid
    enum {n=G::dimension};

    //! get entity from the grid
    typedef typename G::Traits::template Codim<0>::Entity Entity;

  public:
    //! evaluate derivative in local coordinates
    /*! Evaluate the partial derivative a the given position
       in local coordinates in an entity.
       @param[in]  comp    number of component that should be differentiated
       @param[in]  d       vector giving order of derivative for each variable
       @param[in]  e       reference to grid entity of codimension 0
       @param[in]  xi      point in local coordinates of the reference element of e
       \return             value of the derivative
     */
    virtual RT derivativelocal (int comp, const Dune::FieldVector<int,n>& d,
                                const Entity& e, const Dune::FieldVector<DT,n>& xi) const = 0;
  };



  //! Base class for differentiable functions living on a grid
  /*! Functions that are differentiable and live on a grid.

        Template parameters are:
     -  G      a grid class, ctype and dimension are used for DT and n in the base class
     -  RT     type for return values
        -  m      number of components of the vector-valued function
   */
  template<class G, class RT, int m>
  class DifferentiableGridFunctionDefault : virtual public DifferentiableGridFunction<G,RT,m>
  {
    //! get domain field type from the grid
    typedef typename G::ctype DT;

    //! get domain dimension from the grid
    enum {n=G::dimension};

    //! get entity from the grid
    typedef typename G::Traits::template Codim<0>::Entity Entity;

  public:
    //! implement local derivative evaluation with global derivative evaluation
    /*!
       @param[in]  comp    number of component that should be differentiated
       @param[in]  d       vector giving order of derivative for each variable
       @param[in]  e       reference to grid entity of codimension 0
       @param[in]  xi      point in local coordinates of the reference element of e
       \return             value of the derivative
     */
    RT derivativelocal (int comp, const Dune::FieldVector<int,n>& d,
                        const Entity& e, const Dune::FieldVector<DT,n>& xi) const
    {
      return derivative(comp,d,e.geometry().global(xi));
    }
  };



  //! Abstract base class for continuous functions
  /*! A continuous function is derived from function and does not implement
        any of its methods. This class is just there to say that this function
        is continuous.

        Template parameters are:
     -  DT      type for components of the input vector
     -  RT      type for the return values
     -  n       dimension of the input space
     -  m       number of components of the vector-valued function
   */
  template<class DT, class RT, int n, int m>
  class C0Function : virtual public Function<DT,RT,n,m>
  {};

  //! Base class for continuous grid functions
  /*! This class indicates that the grid function is continuous. This
        class is only used to derive from it and to give the derived class
        this property.

        Template parameters are:
     -  G      a grid class, ctype and dimension are used for DT and n in the base class
     -  RT     type for return values
        -  m      number of components of the vector-valued function
   */
  template<class G, class RT, int m>
  class C0GridFunction : virtual public GridFunction<G,RT,m>,
                         virtual public C0Function<typename G::ctype,RT,G::dimension,m>
  {};


  //! Abstract base class for continuously differentiable functions
  /*! This class is derived from function and implements the order method.
        It is just there to indicate that derivatives up to order 1
        of objects of this class are continuous.

        Template parameters are:
     -  DT      type for components of the input vector
     -  RT      type for the return values
     -  n       dimension of the input space
     -  m       number of components of the vector-valued function
   */
  template<class DT, class RT, int n, int m>
  class C1Function : virtual public DifferentiableFunction<DT,RT,n,m>,
                     virtual public C0Function<DT,RT,n,m>
  {
  public:
    //! returns order 1
    /*! For C1 functions we can implement it here.
     */
    virtual int order () const {return 1;}
  };



  //! Abstract base class for L2 functions
  /*! For L2 functions we accept that the return value at a point is not unique.
        This is especially important for L2 functions defined on a grid. There
        the local evaluation might not give the same result as a local
        evaluation even if the position (in global coordinates) is the same.

        Template parameters are:
     -  DT      type for components of the input vector
     -  RT      type for the return values
     -  n       dimension of the input space
     -  m       number of components of the vector-valued function
   */
  template<class DT, class RT, int n, int m>
  class L2Function : virtual public Function<DT,RT,n,m>
  {};



  //! Abstract base class for H1 functions
  /*! Partial derivatives up to order 1 can be taken but the
        pointwise values might not be unique.

        Template parameters are:
     -  DT      type for components of the input vector
     -  RT      type for the return values
     -  n       dimension of the input space
     -  m       number of components of the vector-valued function
   */
  template<class DT, class RT, int n, int m>
  class H1Function : virtual public DifferentiableFunction<DT,RT,n,m>,
                     virtual public L2Function<DT,RT,n,m>
  {
  public:
    //! returns order 1
    /*! For H1 functions we can implement it here.
     */
    virtual int order () const {return 1;}
  };


  //! Abstract base class for Hdiv functions
  /*! The divergence can be taken. But as an L2 function
        its pointwise values might not be unique

        Template parameters are:
     -  DT      type for components of the input vector
     -  RT      type for the return values
     -  n       dimension of the input space
     -  m       number of components of the vector-valued function
   */
  template<class DT, class RT, int n, int m>
  class HdivFunction : virtual public DifferentiableFunction<DT,RT,n,m>,
                       virtual public L2Function<DT,RT,n,m>
  {
  public:
    //! returns order 1
    /*! For Hdiv functions we can implement it here.
     */
    virtual int order () const {return 1;}
  };



  //! Base class for elementwise infinitely differentiable functions
  /*! This class is the base class for typical finite element functions
        which are elementwise polynomials. Using multiple inheritance functions
        can be given several properties.

        Template parameters are:
     -  G      a grid class, ctype and dimension are used for DT and n in the base class
     -  RT     type for return values
        -  m      number of components of the vector-valued function
   */
  template<class G, class RT, int m>
  class ElementwiseCInfinityFunction : virtual public DifferentiableGridFunction<G,RT,m>
  {
  public:
    //! returns order InfinitelyDifferentiable
    /*! Can be implemented already here for this class of functions
     */
    virtual int order () const {return InfinitelyDifferentiable;}
  };



  /** @} */

}
#endif
