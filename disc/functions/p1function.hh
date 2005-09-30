// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef __DUNE_P1FUNCTION_HH__
#define __DUNE_P1FUNCTION_HH__

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
      std::cout << "making FE function with " << mapper_.size() << " components" << std::endl;
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
        value += Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1)[i].evaluateFunction(0,xi)*(*coeff)[mapper_.template map<n>(e,i)];
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
                 *(*coeff)[mapper_.template map<n>(e,i)];
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
      std::vector<bool> visited(mapper_.size());
      for (int i=0; i<mapper_.size(); i++) visited[i] = false;

      Iterator eendit = is.template end<0,All_Partition>();
      for (Iterator it = is.template begin<0,All_Partition>(); it!=eendit; ++it)
      {
        Dune::GeometryType gt = it->geometry().type();
        for (int i=0; i<Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1).size(); ++i)
          if (!visited[mapper_.template map<n>(*it,i)])
          {
            (*coeff)[mapper_.template map<n>(*it,i)][0] =
              u.evallocal(0,*it,Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1)[i].position());
            visited[mapper_.template map<n>(*it,i)] = true;
            //                          std::cout << "evaluated " << it->geometry().global(Dune::LagrangeShapeFunctions<DT,RT,n>::general(gt,1)[i].position())
            //                                            << " value " << (*coeff)[mapper_.template map<n>(*it,i)][0]
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

    /** @brief Initiate update process

        Call this method after the grid has been adapted. The representation is
       now updated to the new grid and the finite element function can be used on
       the new grid. However the data is not initialized.
            The old representation (with respect to the old grid) can still be accessed if
       it has been saved. It is deleted in endUpdate().
     */
    void beginUpdate ()
    {
      oldcoeff = coeff;                   // save the current representation
      mapper_.update();                   // allow mapper to recompute its internal sizes
      try {
        coeff = new RepresentationType(mapper_.size());         // allocate new representation
      }
      catch (std::bad_alloc) {
        std::cerr << "not enough memory in P1FEFunction update" << std::endl;
        throw;         // rethrow exception
      }
      std::cout << "P1 FE function enlarged to " << mapper_.size() << " components" << std::endl;
    }

    /** @brief update function after mesh adaptation

        Note thate this method only
     */
    void endUpdate ()
    {
      // now really delete old representation
      if (oldcoeff!=0) delete oldcoeff;
      oldcoeff = 0;
    }

    //! export a reference to the mapper, this is needed in adaptivity
    const MultipleCodimMultipleGeomTypeMapper<G,IS,P1Layout>& mapper ()
    {
      return mapper_;
    }

    //! export a reference to the grid, this is needed in adaptivity
    const G& grid ()
    {
      return grid_;
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
  template<class G, class RT>
  class LeafP1FEFunction : public P1FEFunction<G,RT,typename G::Traits::LeafIndexSet>
  {
  public:
    LeafP1FEFunction (const G& grid)
      : P1FEFunction<G,RT,typename G::Traits::LeafIndexSet>(grid,grid.leafIndexSet())
    {}
  };


  /** \brief P1 finite element function on a given level grid
   */
  template<class G, class RT>
  class LevelP1FEFunction : public P1FEFunction<G,RT,typename G::Traits::LevelIndexSet>
  {
  public:
    LevelP1FEFunction (const G& grid, int level)
      : P1FEFunction<G,RT,typename G::Traits::LevelIndexSet>(grid,grid.levelIndexSet(level))
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
  public:

    //! manages nothing
    P1FEFunctionManager (const G& g) : savedmap(g)
    {       }

    //! manage one function to start with
    P1FEFunctionManager (FuncType& f) : savedmap(f.grid())
    {
      flist.push_back(&f);
    }

    //! add a function to the list of managed functions
    void enlist (FuncType& f)
    {
      if (flist.empty())
        flist.push_back(&f);
      else
      {
        FuncType* first = *(flist.begin());
        if (&(f.grid()) != &(first->grid()))
          DUNE_THROW(GridError, "tried to register functions on different grids");
        if (f.mapper().size() != first->mapper().size())
          DUNE_THROW(GridError, "tried to register functions with different sizes");
        flist.push_back(&f);
      }
    }

    /** \brief Prepare finite element function for mesh adaptation

        This method has to be called before adapting the grid. It stores information
            with respect to the global ID set of the grid.

            P1 is special in the sense that the data does not have to be processed before
       adaptation. Only the vertex data has to be saved. Processing is done after adaptation
            when the interpolation of the data in new vertices is required. Note that element-wise
       data does require processing before adaptation but nothing after adaptation.
     */
    void preAdapt ()
    {
      // check if any functions are registered
      if (flist.empty()) return;

      // now allocate a vector to store the old indices, the size is known from the mapper
      FuncType& first = *(*(flist.begin()));
      oldindex.resize(first.mapper().size());

      // and allocate the universal mapper to acces the old indices
      const G& grid = first.grid();
      savedmap.clear();     //should be empty already

      // now loop over all vertices and copy the index provided by the mapper
      VLeafIterator veendit = grid.template leafend<dim>();
      for (VLeafIterator it = grid.template leafbegin<dim>(); it!=veendit; ++it)
        oldindex[savedmap.map(*it)] = first.mapper().map(*it);
    }


    /** \brief Adapt the finite element function to the new mesh

        This method must be called after the mesh has been modified. It has be called
       whenever preAdapt has been called.
     */
    void postAdapt ()
    {
      // now we have to copy the data for each finite element function
      for (ListIteratorType lit=flist.begin(); lit!=flist.end(); ++lit)
      {
        // get the players
        FuncType& f = *(*lit);
        RepresentationType& oldcoeff = *f;
        const G& grid = f.grid();

        // update f to the new f but keep old coefficient vector
        f.beginUpdate();

        // now loop over the NEW mesh to copy the data that was already in the OLD mesh
        VLeafIterator veendit = grid.template leafend<dim>();
        for (VLeafIterator it = grid.template leafbegin<dim>(); it!=veendit; ++it)
        {
          // lookup in mapper
          int i;
          if (savedmap.contains(*it,i))
          {
            // the vertex existed already in the old mesh, copy data
            (*f)[f.mapper().map(*it)] = oldcoeff[oldindex[i]];
          }
        }

        // now loop the second time to interpolate the new coefficients
        for (VLeafIterator it = grid.template leafbegin<dim>(); it!=veendit; ++it)
        {
          continue;
          // lookup in mapper
          int i;
          if (!savedmap.contains(*it,i))
          {
            EEntityPointer father=it->ownersFather();
            FieldVector<DT,dim> pos=it->positionInOwnersFather();
            GeometryType gt = father->geometry().type();
            RT value=0;
            for (int i=0; i<Dune::LagrangeShapeFunctions<DT,RT,dim>::general(gt,1).size(); ++i)
            {
              // lookup subid
              int index;
              if (savedmap.template contains<dim>(*father,i,index))
              {
                // the vertex existed already in the old mesh, copy data
                value += Dune::LagrangeShapeFunctions<DT,RT,dim>::general(gt,1)[i].evaluateFunction(0,pos)
                         *oldcoeff[oldindex[index]];
              }
              else
                DUNE_THROW(GridError,"vertex at father element not found");
            }
            (*f)[f.mapper().map(*it)] = value;
          }
        }

        // finish update process, delete old coefficient vector
        f.endUpdate();
      }

      // delete the temporary data
      savedmap.clear();
      oldindex.clear();
    }



  private:
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
