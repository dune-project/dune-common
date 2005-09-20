// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef __DUNE_P1FUNCTION_HH__
#define __DUNE_P1FUNCTION_HH__

#include <iostream>
#include <vector>
#include "common/fvector.hh"
#include "common/exceptions.hh"
#include "grid/common/grid.hh"
#include "grid/common/mcmgmapper.hh"
#include "istl/bvector.hh"
#include "istl/operators.hh"
#include "istl/bcrsmatrix.hh"

#include "functions.hh"
#include "disc/shapefunctions/lagrangeshapefunctions.hh"

/**
 * @file
 * @brief  defines a class for piecewise linear finite element functions
 * @author Peter Bastian
 */
namespace Dune
{
  /** @addtogroup DISC
   *
   * @{
   */
  /**
   * @brief defines a class for piecewise linear finite element functions
   *
   */


  //! class for P1 finite element functions on a grid
  /*! This class implements the interface of a DifferentiableGridFunction
        with piecewise linear elements using a Lagrange basis. It is implemented
        using the general shape functions, thus it should work for all element types
        and dimensions.

        In addition to the DifferentiableGridFunction interface P1 functions can be initialized
        from a C0GridFunction via Lagrange interpolation. Dereferencing delivers
        the coefficient vector.
   */
  template<class G, class RT, typename IS>
  class P1FEFunction : virtual public ElementwiseCInfinityFunction<G,RT,1>,
                       virtual public H1Function<typename G::ctype,RT,G::dimension,1>,
                       virtual public C0GridFunction<G,RT,1>
  {
    //! get domain field type from the grid
    typedef typename G::ctype DT;

    //! get domain dimension from the grid
    enum {n=G::dimension,m=1};

    //! get entity from the grid
    typedef typename G::Traits::template Codim<0>::Entity Entity;

    //! Parameter for mapper class
    template<int dim>
    struct P1Layout
    {
      bool contains (int codim, Dune::GeometryType gt)
      {
        if (codim==dim) return true;
        return false;
      }
    };

  public:
    typedef BlockVector<FieldVector<RT,1> > RepresentationType;

    // make a vector for the grid
    P1FEFunction (const G& g,  const IS& indexset) : grid(g), is(indexset), mapper(g,indexset)
    {
      coeff.resize(mapper.size());
      std::cout << "making FE function with " << mapper.size() << " components" << std::endl;
    }

    //! evaluate single component comp at global point x
    /*! Evaluate a single component of the vector-valued
       function.
       @param[in] comp number of component to be evaluated
       @param[in] x    position to be evaluated
       \return         value of the component
     */
    virtual RT eval (int comp, const Dune::FieldVector<DT,n>& x) const
    {
      DUNE_THROW(NotImplemented, "global eval not implemented yet");
      return 0;
    }

    //! evaluate all components at point x and store result in y
    /*! Evaluation function for all components at once.
       @param[in]  x    position to be evaluated
       @param[out] y    result vector to be filled
     */
    virtual void evalall (const Dune::FieldVector<DT,n>& x, Dune::FieldVector<RT,m>& y) const
    {
      DUNE_THROW(NotImplemented, "global eval not implemented yet");
    }

    //! evaluate partial derivative
    /*! Evaluate partial derivative of a component of the vector-valued function.
       @param[in]  comp    number of component that should be differentiated
       @param[in]  d       vector giving order of derivative for each variable
       @param[in]  x       position where derivative is to be evaluated
       \return             value of the derivative
     */
    virtual RT derivative (int comp, const Dune::FieldVector<int,n>& d, const Dune::FieldVector<DT,n>& x) const
    {
      DUNE_THROW(NotImplemented, "global derivative not implemented yet");
    }

    //! return number of partial derivatives that can be taken
    /*! A DifferentiableFunction can say how many derivatives exist
       and can be safely evaluated.
     */
    virtual int order () const
    {
      return 1;     // up to now only one derivative is implemented
    }

    //! evaluate single component comp in the entity e at local coordinates xi
    /*! Evaluate the function in an entity at local coordinates.
       @param[in]  comp   number of component to be evaluated
       @param[in]  e      reference to grid entity of codimension 0
       @param[in]  xi     point in local coordinates of the reference element of e
       \return            value of the component
     */
    virtual RT evallocal (int comp, const Entity& e, const Dune::FieldVector<DT,n>& xi) const
    {
      RT value=0;
      Dune::GeometryType gt = e.geometry().type();     // extract type of element
      for (int i=0; i<Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1).size(); ++i)
        value += Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1)[i].evaluateFunction(0,xi)*coeff[mapper.template submap<n>(e,i)];
      return value;
    }

    //! evaluate all components  in the entity e at local coordinates xi
    /*! Evaluates all components of a function at once.
       @param[in]  e      reference to grid entity of codimension 0
       @param[in]  xi     point in local coordinates of the reference element of e
       @param[out] y      vector with values to be filled
     */
    virtual void evalalllocal (const Entity& e, const Dune::FieldVector<DT,G::dimension>& xi,
                               Dune::FieldVector<RT,m>& y) const
    {
      y[0] = evallocal(0,e,xi);
    }

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
                                const Entity& e, const Dune::FieldVector<DT,n>& xi) const
    {
      int dir;
      int order=0;
      for (int i=0; i<n; i++)
      {
        order += d[i];
        if (d[i]>0) dir=i;
      }
      if (order!=1) DUNE_THROW(GridError,"can only evaluate one derivative");

      RT value=0;
      Dune::GeometryType gt = e.geometry().type();     // extract type of element
      for (int i=0; i<Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1).size(); ++i)
        value += Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1)[i].evaluateDerivative(0,dir,xi)
                 *coeff[mapper.template submap<n>(e,i)];
      return value;
    }


    //! interpolate nodal values from a grid function
    /*! Lagrange interpolation of a P1 finite element function from given
       continuous grid function. Evaluation is done by visiting the vertices
       of each element and storing a bitvector of visited vertices.

       @param[in]  u    a continuous grid function
     */
    void interpolate (const C0GridFunction<G,RT,1>& u)
    {
      typedef typename IS::template Codim<0>::template Partition<All_Partition>::Iterator Iterator;
      std::vector<bool> visited(mapper.size());
      for (int i=0; i<mapper.size(); i++) visited[i] = false;

      Iterator eendit = is.template end<0,All_Partition>();
      for (Iterator it = is.template begin<0,All_Partition>(); it!=eendit; ++it)
      {
        Dune::GeometryType gt = it->geometry().type();
        for (int i=0; i<Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1).size(); ++i)
          if (!visited[mapper.template submap<n>(*it,i)])
          {
            coeff[mapper.template submap<n>(*it,i)][0] =
              u.evallocal(0,*it,Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1)[i].position());
            visited[mapper.template submap<n>(*it,i)] = true;
            //                          std::cout << "evaluated " << it->geometry().global(Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1)[i].position())
            //                                            << " value " << coeff[mapper.template submap<n>(*it,i)][0]
            //                                            << std::endl;
          }
      }
    }

    //! return const reference to coefficient vector
    /*! Dereferencing a finite element function returns the
       coefficient representation of the finite element function.
       This is the const version.
     */
    const RepresentationType& operator* () const
    {
      return coeff;
    }

    //! return reference to coefficient vector
    /*! Dereferencing a finite element function returns the
       coefficient representation of the finite element function.
       This is the non-const version.
     */
    RepresentationType& operator* ()
    {
      return coeff;
    }

  private:
    // a reference to the grid
    const G& grid;

    // reference to index set on the grid (might be level or leaf)
    const IS& is;

    // we need a mapper
    MultipleCodimMultipleGeomTypeMapper<G,IS,P1Layout> mapper;

    // and a vector
    RepresentationType coeff;            // the coefficient vector
  };


  template<class G, class RT>
  class LeafP1FEFunction : public P1FEFunction<G,RT,typename G::Traits::LeafIndexSet>
  {
  public:
    LeafP1FEFunction (const G& grid)
      : P1FEFunction<G,RT,typename G::Traits::LeafIndexSet>(grid,grid.leafIndexSet())
    {}
  };


  template<class G, class RT>
  class LevelP1FEFunction : public P1FEFunction<G,RT,typename G::Traits::LevelIndexSet>
  {
  public:
    LevelP1FEFunction (const G& grid, int level)
      : P1FEFunction<G,RT,typename G::Traits::LevelIndexSet>(grid,grid.levelIndexSet(level))
    {}
  };



  /** @} */

}
#endif
