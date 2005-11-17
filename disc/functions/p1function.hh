// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_P1FUNCTION_HH
#define DUNE_P1FUNCTION_HH

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
#include "p0function.hh"

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

  // forward declaration
  template<class G, class RT> class P1FEFunctionManager;


  //! class for P1 finite element functions on a grid
  /*! This class implements the interface of a DifferentiableGridFunction
        with piecewise linear elements using a Lagrange basis. It is implemented
        using the general shape functions, thus it should work for all element types
        and dimensions.

        In addition to the DifferentiableGridFunction interface P1 functions can be initialized
        from a C0GridFunction via Lagrange interpolation. Dereferencing delivers
        the coefficient vector.
   */
  template<class G, class RT, typename IS, int m=1>
  class P1FEFunction : virtual public ElementwiseCInfinityFunction<G,RT,m>,
                       virtual public H1Function<typename G::ctype,RT,G::dimension,m>,
                       virtual public C0GridFunction<G,RT,m>
  {
    //! get domain field type from the grid
    typedef typename G::ctype DT;

    //! get domain dimension from the grid
    enum {n=G::dimension};

    //! get entity from the grid
    typedef typename G::template Codim<0>::Entity Entity;

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

    //! make copy constructor private
    P1FEFunction (const P1FEFunction&);

  public:
    typedef FieldVector<RT,m> BlockType;
    typedef BlockVector<BlockType> RepresentationType;

    //! allocate a vector with the data
    P1FEFunction (const G& g,  const IS& indexset) : grid_(g), is(indexset), mapper_(g,indexset)
    {
      oldcoeff = 0;
      try {
        coeff = new RepresentationType(mapper_.size());
      }
      catch (std::bad_alloc) {
        std::cerr << "not enough memory in P1FEFunction" << std::endl;
        throw;         // rethrow exception
      }
      dverb << "making FE function with " << mapper_.size() << " components" << std::endl;
    }

    //! deallocate the vector
    ~P1FEFunction ()
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
        value += Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1)[i].evaluateFunction(0,xi)*(*coeff)[mapper_.template map<n>(e,i)][comp];
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
      Dune::GeometryType gt = e.geometry().type();     // extract type of element
      y = 0;
      for (int i=0; i<Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1).size(); ++i)
      {
        RT basefuncvalue=Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1)[i].evaluateFunction(0,xi);
        int index = mapper_.template map<n>(e,i);
        for (int c=0; c<m; c++)
          y[c] += basefuncvalue * (*coeff)[index][c];
      }
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
      int dir=-1;
      int order=0;
      for (int i=0; i<n; i++)
      {
        order += d[i];
        if (d[i]>0) dir=i;
      }
      assert(dir != -1);
      if (order!=1) DUNE_THROW(GridError,"can only evaluate one derivative");

      RT value=0;
      Dune::GeometryType gt = e.geometry().type();     // extract type of element
      for (int i=0; i<Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1).size(); ++i)
        value += Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1)[i].evaluateDerivative(0,dir,xi)
                 *(*coeff)[mapper_.template map<n>(e,i)][comp];
      return value;
    }


    //! interpolate nodal values from a grid function
    /*! Lagrange interpolation of a P1 finite element function from given
       continuous grid function. Evaluation is done by visiting the vertices
       of each element and storing a bitvector of visited vertices.

       @param[in]  u    a continuous grid function
     */
    void interpolate (const C0GridFunction<G,RT,m>& u)
    {
      typedef typename IS::template Codim<0>::template Partition<All_Partition>::Iterator Iterator;
      std::vector<bool> visited(mapper_.size());
      for (int i=0; i<mapper_.size(); i++) visited[i] = false;

      Iterator eendit = is.template end<0,All_Partition>();
      for (Iterator it = is.template begin<0,All_Partition>(); it!=eendit; ++it)
      {
        Dune::GeometryType gt = it->geometry().type();
        for (int i=0; i<Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1).size(); ++i)
          if (!visited[mapper_.template map<n>(*it,i)])
          {
            for (int c=0; c<m; c++)
              (*coeff)[mapper_.template map<n>(*it,i)][c] =
                u.evallocal(c,*it,Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1)[i].position());
            visited[mapper_.template map<n>(*it,i)] = true;
          }
      }
    }

    void interpolate (const P0FEFunction<G,RT,IS,m>& u)
    {
      typedef typename IS::template Codim<0>::template Partition<All_Partition>::Iterator Iterator;
      std::vector<char> counter(mapper_.size());
      for (int i=0; i<mapper_.size(); i++) counter[i] = 0;

      for (int i=0; i<counter.size(); i++)
        (*coeff)[i] = 0;
      Iterator eendit = is.template end<0,All_Partition>();
      for (Iterator it = is.template begin<0,All_Partition>(); it!=eendit; ++it)
      {
        Dune::GeometryType gt = it->geometry().type();
        for (int i=0; i<Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1).size(); ++i)
        {
          for (int c=0; c<m; c++)
            (*coeff)[mapper_.template map<n>(*it,i)][c] +=
              u.evallocal(c,*it,Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1)[i].position());
          counter[mapper_.template map<n>(*it,i)] += 1;
        }
      }
      for (int i=0; i<counter.size(); i++)
        for (int c=0; c<m; c++)
          (*coeff)[i][c] /= counter[i];
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
    {}

    /** @brief Initiate update process

        Call this method after the grid has been adapted. The representation is
       now updated to the new grid and the finite element function can be used on
       the new grid. However the data is not initialized.
            The old representation (with respect to the old grid) can still be accessed if
       it has been saved. It is deleted in endUpdate().
     */
    void postAdapt (P1FEFunctionManager<G,RT>& manager)
    {
      typedef typename G::template Codim<n>::LeafIterator VLeafIterator;
      typedef typename G::template Codim<0>::LeafIterator ELeafIterator;
      typedef typename G::template Codim<n>::LevelIterator VLevelIterator;
      typedef typename G::template Codim<0>::LevelIterator ELevelIterator;
      typedef typename G::template Codim<0>::EntityPointer EEntityPointer;

      // \todo check that function is only called for data with respect to leafs
      // save the current representation
      oldcoeff = coeff;

      // allow mapper to recompute its internal sizes
      mapper_.update();

      // allocate data with new size (while keeping the old data ...)
      try {
        coeff = new RepresentationType(mapper_.size());         // allocate new representation
      }
      catch (std::bad_alloc) {
        std::cerr << "not enough memory in P1FEFunction update" << std::endl;
        throw;         // rethrow exception
      }
      std::cout << "P1 FE function enlarged to " << mapper_.size() << " components" << std::endl;

      // vector of flags to store which vertex has been handled already
      std::vector<bool> visited(mapper_.size());
      for (int i=0; i<mapper_.size(); i++) visited[i] = false;

      // now loop over the NEW mesh to copy the data that was already in the OLD mesh
      VLeafIterator veendit = grid_.template leafend<n>();
      for (VLeafIterator it = grid_.template leafbegin<n>(); it!=veendit; ++it)
      {
        // lookup in mapper
        int i;
        if (manager.savedMap().contains(*it,i))
        {
          //                      std::cout << " found vertex=" << it->geometry()[0]
          //                                            << " at i=" << i
          //                                            << " oldindex=" << manager.oldIndex()[i]
          //                                            << " newindex=" << mapper_.map(*it)
          //                                            << std::endl;
          // the vertex existed already in the old mesh, copy data
          for (int c=0; c<m; c++)
            (*coeff)[mapper_.map(*it)][c] = (*oldcoeff)[manager.oldIndex()[i]][c];
          // ... and mark as visited
          visited[mapper_.map(*it)] = true;
        }
      }

      // now loop the second time to interpolate the new coefficients
      // new implementation using interpolation on codim 0
      for (int level=1; level<=grid_.maxLevel(); ++level)
      {
        ELevelIterator elendit = grid_.template lend<0>(level);
        for (ELevelIterator it = grid_.template lbegin<0>(level); it!=elendit; ++it)
        {
          GeometryType gte = it->geometry().type();
          for (int i=0; i<Dune::LagrangeShapeFunctions<DT,RT,n>::general(gte,1).size(); ++i)
          {
            int index = mapper_.template map<n>(*it,i);
            if (!visited[index])
            {
              // OK, this is a new vertex
              EEntityPointer father=it->father();                             // the father element
              GeometryType gtf = father->geometry().type();                             // fathers type
              const FieldVector<DT,n>& cpos=Dune::LagrangeShapeFunctions<DT,RT,n>::general(gte,1)[i].position();
              FieldVector<DT,n> pos = it->geometryInFather().global(cpos);                             // map corner to father element
              for (int c=0; c<m; c++)
                (*coeff)[index][c] = 0;
              for (int j=0; j<Dune::LagrangeShapeFunctions<DT,RT,n>::general(gtf,1).size(); ++j)
              {
                RT basefuncvalue = Dune::LagrangeShapeFunctions<DT,RT,n>::general(gtf,1)[j].evaluateFunction(0,pos);
                for (int c=0; c<m; c++)
                  (*coeff)[index][c] += basefuncvalue * (*coeff)[mapper_.template map<n>(*father,j)][c];
                //                                        std::cout << "  corner=" << i
                //                                                              << " cpos=" << father->geometry()[i]
                //                                                              << " u=" << (*coeff)[mapper_.template map<n>(*father,i)]
                //                                                              << std::endl;
              }
              //                                  std::cout << "index=" << mapper_.map(*it) << " value=" << value << std::endl;
              visited[index] = true;
            }
          }
        }
      }

      // now really delete old representation
      if (oldcoeff!=0) delete oldcoeff;
      oldcoeff = 0;
    }

    /** @brief export the mapper for external use
     */
    const MultipleCodimMultipleGeomTypeMapper<G,IS,P1Layout>& mapper () const
    {
      return mapper_;
    }

  private:
    // a reference to the grid
    const G& grid_;

    // reference to index set on the grid (might be level or leaf)
    const IS& is;

    // we need a mapper
    MultipleCodimMultipleGeomTypeMapper<G,IS,P1Layout> mapper_;

    // and a dynamically allocated vector
    RepresentationType* coeff;

    // saved pointer in update phase
    RepresentationType* oldcoeff;
  };


  /** \brief P1 finite element function on the leaf grid
   */
  template<class G, class RT, int m=1>
  class LeafP1FEFunction : public P1FEFunction<G,RT,typename G::template Codim<0>::LeafIndexSet,m>
  {
  public:
    LeafP1FEFunction (const G& grid)
      : P1FEFunction<G,RT,typename G::template Codim<0>::LeafIndexSet,m>(grid,grid.leafIndexSet())
    {}
  };


  /** \brief P1 finite element function on a given level grid
   */
  template<class G, class RT, int m=1>
  class LevelP1FEFunction : public P1FEFunction<G,RT,typename G::template Codim<0>::LevelIndexSet,m>
  {
  public:
    LevelP1FEFunction (const G& grid, int level)
      : P1FEFunction<G,RT,typename G::template Codim<0>::LevelIndexSet,m>(grid,grid.levelIndexSet(level))
    {}
  };


  /** \brief Manage mesh adaptation and load balancing for several P1 finite element functions

      Adaptivity management is only required for the leaf finite element functions,
          therefore we do only allow those to be registered.

          There is still a problem: If we would have P1 vector valued functions with different
      numbers of components we still would need a seperate manager for every size, although
      this is actually not necessary.
   */
  template<class G, class RT>
  class P1FEFunctionManager {
    enum {dim=G::dimension};
    typedef typename G::ctype DT;
    typedef LeafP1FEFunction<G,RT> FuncType;
    typedef typename LeafP1FEFunction<G,RT>::RepresentationType RepresentationType;
    typedef std::list<FuncType*> ListType;
    typedef typename std::list<FuncType*>::iterator ListIteratorType;
    typedef typename G::template Codim<dim>::LeafIterator VLeafIterator;
    typedef typename G::template Codim<0>::EntityPointer EEntityPointer;

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

    //! manages nothing
    P1FEFunctionManager (const G& g) : grid(g), savedmap(g), mapper(g,g.leafIndexSet())
    {
      // allocate index array to correct size (this possible for vertex data)
      oldindex.resize(mapper.size());

      // and allocate the universal mapper to acces the old indices
      savedmap.clear();     //should be empty already

      // now loop over all vertices and copy the index provided by the mapper
      VLeafIterator veendit = grid.template leafend<dim>();
      for (VLeafIterator it = grid.template leafbegin<dim>(); it!=veendit; ++it)
      {
        oldindex[savedmap.map(*it)] = mapper.map(*it);
      }
    }

    const GlobalUniversalMapper<G>& savedMap ()
    {
      return savedmap;
    }

    const std::vector<int>& oldIndex ()
    {
      return oldindex;
    }

  private:
    // we need a mapper
    MultipleCodimMultipleGeomTypeMapper<G,typename G::template Codim<0>::LeafIndexSet,P1Layout> mapper;

    // store a reference to the grid that is managed
    const G& grid;

    // maintain a list of registered functions
    ListType flist;

    // We need a persistent consecutive enumeration
    GlobalUniversalMapper<G> savedmap;

    // The old leaf indices are stored in a dynamically allocated vector
    std::vector<int> oldindex;
  };


  /** @} */

}
#endif
