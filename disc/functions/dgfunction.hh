// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DG_FUNCTION_HH
#define DG_FUNCTION_HH

#include <dune/istl/bvector.hh>
#include <dune/disc/functions/functions.hh>
#include <dune/disc/shapefunctions/dgspace/monomialshapefunctions.hh>

/**
 * @file dune/disc/functions/dgfunction.hh
 * @brief a class for discontinous galerkin element functions
 * @author Christian Engwer
 */

namespace Dune {

  /** @addtogroup DISC_Functions
   *
   * @{
   */

  //! class for DG finite element functions on a grid
  /*! This class implements the interface of a
        DifferentiableGridFunction with piecewise discontinous elements
        using a monomial basis. It is assumed that all elements use the
        same shapefunction set.

        In addition to the DifferentiableGridFunction
        interface. Dereferencing delivers the coefficient vector.

     \tparam G The grid
     \tparam RT The type used for the component values of the function
     \tparam IS The IndexSet associating the values of the funciton with
               according entities
     \tparam o The order of the monomial shapefunctions
   */
  template<class G, class RT, class IS, int o>
  class DGFunction :
    virtual public GridFunction<G,RT,1>,
    virtual public GridFunctionDefault<G,RT,1>,
    virtual public GridFunctionGlobalEvalDefault<G,RT,IS,1>,
    virtual public FunctionDefault<typename G::ctype,RT,G::dimension,1>,
    virtual public L2Function<typename G::ctype,RT,G::dimension,1>
  {
    //! get domain field type from the grid
    typedef typename G::ctype DT;

    //! get entity from the grid
    typedef typename G::template Codim<0>::Entity Entity;

    //! get domain dimension from the grid
    enum {n=G::dimension};

    //! make copy constructor private
    DGFunction (const DGFunction&);

    //! size of local vector blocks
    static const int BlockSize =
      MonomialShapeFunctionSetSize<dim,o>::maxSize;
    //! type of shapefunctions
    typedef Dune::MonomialShapeFunctionSetContainer<DT,RT,G::dimension,o>
    ShapeFunctionSetContainer;
    typedef Dune::MonomialShapeFunctionSet<DT,RT,G::dimension>
    ShapeFunctionSet;
  public:
    typedef RT ResultType;
    typedef DT DomainType;
    typedef
    FieldVector<ResultType,BlockSize> BlockType;
    typedef BlockVector< BlockType > RepresentationType;

    //! allocate a vector with the data
    DGFunction (const G& g, const IS& indexset) :
      GridFunctionGlobalEvalDefault<G,RT,IS,1>(g,indexset), grid_(g), is_(indexset)
    {
      coeff_.resize(is_.size(0), false);
    }

    //! deallocate the vector
    ~DGFunction () {}

    //! evaluate single component comp at global point x
    /*! Evaluate a single component of the vector-valued
       function.
       @param[in] comp number of component to be evaluated
       @param[in] x    position to be evaluated
       \return         value of the component
     */
    //! evaluate single component comp at global point x
    /*! Evaluate a single component of the vector-valued
       function.
       @param[in] comp number of component to be evaluated
       @param[in] x    position to be evaluated
       \return         value of the component
     */
    //  virtual RT eval (int comp, const Dune::FieldVector<DT,n>& x) const
    //  {
    //    DUNE_THROW(NotImplemented, "global eval not implemented yet");
    //    return 0;
    //  }

    //! evaluate single component comp in the entity e at local coordinates xi
    /*! Evaluate the function in an entity at local coordinates.
       @param[in]  comp   number of component to be evaluated
       @param[in]  e      reference to grid entity of codimension 0
       @param[in]  xi     point in local coordinates of the reference element of e
       \return            value of the component
     */
    virtual RT evallocal (int comp, const Entity& e, const Dune::FieldVector<DT,n>& xi) const
    {
      assert(comp == 0);

      RT value = 0;
      const ShapeFunctionSet & s = shapefnkts_(e.geometry().type(),o);
      int eid = is_.index(e);
      for (int i=0; i<BlockSize; ++i)
      {
        value += coeff_[eid][i] * s[i].evaluateFunction(0, xi);
      }
      return value;
    }

    //! interpolate nodal values from a grid function
    /*! Lagrange interpolation of a P0 finite element function from given
       continuous grid function. Evaluation is done by visiting the vertices
       of each element and storing a bitvector of visited vertices.

       @param[in]  u    a continuous grid function
     */
    void interpolate (const C0GridFunction<G,RT,1>& u)
    {
      DUNE_THROW(NotImplemented, "interpolate(C0GridFunction)");
    }

    //! return const reference to coefficient vector
    /*! Dereferencing a finite element function returns the
       coefficient representation of the finite element function.
       This is the const version.
     */
    const RepresentationType& operator* () const
    {
      return coeff_;
    }

    //! return reference to coefficient vector
    /*! Dereferencing a finite element function returns the
       coefficient representation of the finite element function.
       This is the non-const version.
     */
    RepresentationType& operator* ()
    {
      return coeff_;
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

        Call this method after the grid has been adapted. The
       representation is now updated to the new grid and the finite
       element function can be used on the new grid. However the data
       is not initialized.  The old representation (with respect to
       the old grid) can still be accessed if it has been saved. It
       is deleted in endUpdate().
     */
    void postAdapt ()
    {
      DUNE_THROW(NotImplemented, "postAdapt()");
    }

  private:
    // a reference to the grid
    const G& grid_;

    // reference to index set on the grid (might be level or leaf)
    const IS& is_;

    // reference to local shapefunctionset
    ShapeFunctionSetContainer shapefnkts_;

    // and a dynamically allocated vector
    RepresentationType coeff_;
  };

  /**
     \brief DG finite element function on the leaf grid

     \tparam G The grid
     \tparam RT The type used for the component values of the function
     \tparam o The order of the monomial shapefunctions
   */
  template<class G, class RT, int o>
  class LeafDGFunction : public DGFunction<G,RT,typename G::template Codim<0>::LeafIndexSet,o>
  {
  public:
    LeafDGFunction (const G& grid)
      : DGFunction<G,RT,typename G::template Codim<0>::LeafIndexSet,o>(grid,grid.leafIndexSet())
    {}
  };

  /**
     \brief DG finite element function on a given level grid

     \tparam G The grid
     \tparam RT The type used for the component values of the function
     \tparam o The order of the monomial shapefunctions
   */
  template<class G, class RT, int o>
  class LevelDGFunction : public DGFunction<G,RT,typename G::template Codim<0>::LevelIndexSet,o>
  {
  public:
    LevelDGFunction (const G& grid, int level)
      : DGFunction<G,RT,typename G::template Codim<0>::LevelIndexSet,o>(grid,grid.levelIndexSet(level))
    {}
  };

  /** @} */

}

#endif // DG_FUNCTION_HH
