// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_P0FUNCTION_HH
#define DUNE_P0FUNCTION_HH

//C++ includes
#include <new>
#include <iostream>
#include <vector>
#include <list>
#include <map>

// Dune includes
#include "dune/common/fvector.hh"
#include "dune/common/exceptions.hh"
#include "dune/grid/common/grid.hh"
#include "dune/grid/common/mcmgmapper.hh"
#include "dune/grid/common/universalmapper.hh"
#include "dune/istl/bvector.hh"
#include "dune/istl/operators.hh"
#include "dune/istl/bcrsmatrix.hh"
#include "dune/disc/shapefunctions/lagrangeshapefunctions.hh"

// same directory includes
#include "functions.hh"

/**
 * @file dune/disc/functions/p0function.hh
 * @brief  defines a class for piecewise linear finite element functions
 * @author Peter Bastian
 */
namespace Dune
{
  /** @addtogroup DISC_Functions
   *
   * @{
   */
  /**
   * @brief defines a class for piecewise linear finite element functions
   *
   */

  // forward declaration
  //  template<class G, class RT> class P0FEFunctionManager;


  //! class for P0 finite element functions on a grid
  /*! This class implements the interface of a DifferentiableGridFunction
        with piecewise linear elements using a Lagrange basis. It is implemented
        using the general shape functions, thus it should work for all element types
        and dimensions.

        In addition to the DifferentiableGridFunction interface P0 functions can be initialized
        from a C0GridFunction via Lagrange interpolation. Dereferencing delivers
        the coefficient vector.
   */
  template<class G, class RT, typename IS, int m=1>
  class P0Function : virtual public GridFunction<G,RT,m>,
                     virtual public L2Function<typename G::ctype,RT,G::dimension,m>
  {
    //! get domain field type from the grid
    typedef typename G::ctype DT;

    //! get domain dimension from the grid
    enum {n=G::dimension};

    //! get entity from the grid
    typedef typename G::template Codim<0>::Entity Entity;

    //! Parameter for mapper class
    template<int dim>
    struct P0Layout
    {
      bool contains (int codim, Dune::GeometryType gt)
      {
        if (codim==0) return true;
        return false;
      }
    };

    //! make copy constructor private
    P0Function (const P0Function&);

  public:
    typedef BlockVector<FieldVector<RT,m> > RepresentationType;

    //! allocate a vector with the data
    P0Function (const G& g,  const IS& indexset) : grid_(g), is(indexset), mapper_(g,indexset)
    {
      oldcoeff = 0;
      try {
        coeff = new RepresentationType(mapper_.size());
      }
      catch (std::bad_alloc) {
        std::cerr << "not enough memory in P0Function" << std::endl;
        throw;         // rethrow exception
      }
      std::cout << "making  function with " << mapper_.size() << " components" << std::endl;
    }

    //! deallocate the vector
    ~P0Function ()
    {
      delete coeff;
      if (oldcoeff!=0) delete oldcoeff;
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

    //! evaluate single component comp in the entity e at local coordinates xi
    /*! Evaluate the function in an entity at local coordinates.
       @param[in]  comp   number of component to be evaluated
       @param[in]  e      reference to grid entity of codimension 0
       @param[in]  xi     point in local coordinates of the reference element of e
       \return            value of the component
     */
    virtual RT evallocal (int comp, const Entity& e, const Dune::FieldVector<DT,n>& xi) const
    {
      return (*coeff)[mapper_.map(e)][comp];
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
      for (int c=0; c<m; c++)
        y[c] = (*coeff)[mapper_.map(e)][c];
    }


    //! interpolate nodal values from a grid function
    /*! Lagrange interpolation of a P0 finite element function from given
       continuous grid function. Evaluation is done by visiting the vertices
       of each element and storing a bitvector of visited vertices.

       @param[in]  u    a continuous grid function
     */
    void interpolate (const C0GridFunction<G,RT,m>& u)
    {
      typedef typename IS::template Codim<0>::template Partition<All_Partition>::Iterator Iterator;

      Iterator eendit = is.template end<0,All_Partition>();
      for (Iterator it = is.template begin<0,All_Partition>(); it!=eendit; ++it)
      {
        Dune::GeometryType gt = it->geometry().type();
        for (int c=0; c<m; c++)
          (*coeff)[mapper_.map(*it)][c] =
            u.evallocal(c,*it,Dune::ReferenceElements<DT,n>::general(gt).position(0,0));
      }
    }

    //! return const reference to coefficient vector
    /*! Dereferencing a finite element function returns the
       coefficient representation of the finite element function.
       This is the const version.
     */
    const RepresentationType& operator* () const
    {
      return (*coeff);
    }

    //! return reference to coefficient vector
    /*! Dereferencing a finite element function returns the
       coefficient representation of the finite element function.
       This is the non-const version.
     */
    RepresentationType& operator* ()
    {
      return (*coeff);
    }

    /** empty method to maintain symmetry
            For vertex data nothing is required in preAdapt but for other
       finite element functions this method is necessary.
     */
    void preAdapt ()
    {
      DUNE_THROW(NotImplemented, "preAdapt()");
    }

    /** @brief Initiate update process

        Call this method after the grid has been adapted. The representation is
       now updated to the new grid and the finite element function can be used on
       the new grid. However the data is not initialized.
            The old representation (with respect to the old grid) can still be accessed if
       it has been saved. It is deleted in endUpdate().
     */
    void postAdapt ()
    {
      DUNE_THROW(NotImplemented, "postAdapt()");
    }

    /** @brief export the mapper for external use
     */
    const MultipleCodimMultipleGeomTypeMapper<G,IS,P0Layout>& mapper () const
    {
      return mapper_;
    }

  private:
    // a reference to the grid
    const G& grid_;

    // reference to index set on the grid (might be level or leaf)
    const IS& is;

    // we need a mapper
    MultipleCodimMultipleGeomTypeMapper<G,IS,P0Layout> mapper_;

    // and a dynamically allocated vector
    RepresentationType* coeff;

    // saved pointer in update phase
    RepresentationType* oldcoeff;
  };


  /** \brief P0 finite element function on the leaf grid
   */
  template<class G, class RT, int m=1>
  class LeafP0Function : public P0Function<G,RT,typename G::template Codim<0>::LeafIndexSet,m>
  {
  public:
    LeafP0Function (const G& grid)
      : P0Function<G,RT,typename G::template Codim<0>::LeafIndexSet,m>(grid,grid.leafIndexSet())
    {}
  };


  /** \brief P0 finite element function on a given level grid
   */
  template<class G, class RT, int m=1>
  class LevelP0Function : public P0Function<G,RT,typename G::template Codim<0>::LevelIndexSet,m>
  {
  public:
    LevelP0Function (const G& grid, int level)
      : P0Function<G,RT,typename G::template Codim<0>::LevelIndexSet,m>(grid,grid.levelIndexSet(level))
    {}
  };



  /*! wrap any vector to use it as a P0 function
   */
  template<class G, class T>
  class LeafP0FunctionWrapper : virtual public GridFunction<G,typename T::value_type,1>,
                                virtual public L2Function<typename G::ctype,typename T::value_type,G::dimension,1>
  {
    //! get domain field type from the grid
    typedef typename G::ctype DT;
    typedef typename T::value_type RT;
    typedef typename G::template Codim<0>::LeafIndexSet IS;
    enum {m=1};

    //! get domain dimension from the grid
    enum {n=G::dimension};

    //! get entity from the grid
    typedef typename G::template Codim<0>::Entity Entity;

    //! Parameter for mapper class
    template<int dim>
    struct P0Layout
    {
      bool contains (int codim, Dune::GeometryType gt)
      {
        if (codim==0) return true;
        return false;
      }
    };

    //! make some methods private
    LeafP0FunctionWrapper (const LeafP0FunctionWrapper&);
    LeafP0FunctionWrapper& operator= (const LeafP0FunctionWrapper&);

  public:
    typedef T RepresentationType;

    //! allocate a vector with the data
    LeafP0FunctionWrapper (const G& g, const T& v)
      : grid(g), is(g.leafIndexSet()), mapper(g,g.leafIndexSet()), coeff(v)
    {
      if (mapper.size()!=v.size())
        DUNE_THROW(MathError,"LeafP0FunctionWrapper: size of vector does not match grid size");

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

    //! evaluate single component comp in the entity e at local coordinates xi
    /*! Evaluate the function in an entity at local coordinates.
       @param[in]  comp   number of component to be evaluated
       @param[in]  e      reference to grid entity of codimension 0
       @param[in]  xi     point in local coordinates of the reference element of e
       \return            value of the component
     */
    virtual RT evallocal (int comp, const Entity& e, const Dune::FieldVector<DT,n>& xi) const
    {
      return coeff[mapper.map(e)];
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
      y[0] = coeff[mapper.map(e)];
    }

  private:
    // a reference to the grid
    const G& grid;

    // reference to index set on the grid (might be level or leaf)
    const IS& is;

    // we need a mapper
    MultipleCodimMultipleGeomTypeMapper<G,IS,P0Layout> mapper;

    // and a dynamically allocated vector
    const T& coeff;
  };


  /** @} */

}
#endif
