// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONE_D_GRID_ENTITY_HH
#define DUNE_ONE_D_GRID_ENTITY_HH

#include <dune/common/fixedarray.hh>

#include "onedgridelement.hh"

/** \file
 * \brief The OneDGridEntity class and its specializations
 */

namespace Dune {

  //**********************************************************************
  //
  // --OneDGridEntity
  // --Entity
  //
  /** \brief The implementation of entities in a OneDGrid
     \ingroup OneDGrid

     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

   */
  template<int codim, int dim, class GridImp>
  class OneDGridEntity :
    public EntityDefault <codim,dim,GridImp,OneDGridEntity>
  {

    template <int codim_, PartitionIteratorType PiType_, class GridImp_>
    friend class OneDGridLevelIterator;

    friend class OneDGrid<dim,GridImp::dimensionworld>;

    //! Constructor with a given grid level
    OneDGridEntity(int level, double coord) : geo_(coord), level_(level), pred_(NULL), succ_(NULL) {}

  public:
    typedef typename GridImp::template codim<codim>::Geometry Geometry;
    typedef typename GridImp::template codim<codim>::LevelIterator LevelIterator;
    typedef typename GridImp::template codim<0>::IntersectionIterator IntersectionIterator;
    typedef typename GridImp::template codim<0>::HierarchicIterator HierarchicIterator;

    //! level of this element
    int level () const {return level_;}

    //! index is unique and consecutive per level and codim
    //! used for access to degrees of freedom
    int index () const {return index_;}

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
       with codimension cc.
     */
    //!< Default codim 1 Faces and codim == dim Vertices
    template<int cc> int count () const;

    //! return index of sub entity with codim = cc and local number i
    //! i.e. return global number of vertex i
    /** \todo So far only implemented for cc==dim */
    template<int cc> int subIndex (int i) const;

    //! Provide access to mesh entity i of given codimension. Entities
    //!  are numbered 0 ... count<cc>()-1
    template<int cc>
    OneDGridLevelIterator<cc,All_Partition, GridImp> entity (int i);

    //! geometry of this entity
    const Geometry& geometry () const {return geo_;}

    /** \brief Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
     *
       This can speed up on-the-fly interpolation for linear conforming elements
       Possibly this is sufficient for all applications we want on-the-fly.
     */
    FieldVector<OneDCType, dim>& local ();



    //private:

    //! the current geometry
    OneDMakeableGeometry<dim-codim,dim,GridImp> geo_;

    FieldVector<OneDCType, dim> localFatherCoords_;

    //! entity number
    int index_;

    //! level
    int level_;

  public:
    //!
    OneDGridEntity<codim,dim,GridImp>* pred_;

    OneDGridEntity<codim,dim,GridImp>* succ_;


  };

  //***********************
  //
  //  --OneDGridEntity
  //  --0Entity
  //
  //***********************



  /** \brief Specialization for codim-0-entities.
   * \ingroup OneDGrid
   *
   * This class embodies the topological parts of elements of the grid.
   * It has an extended interface compared to the general entity class.
   * For example, Entities of codimension 0  allow to visit all neighbors.
   *
   * OneDGrid only implements the case dim==dimworld==1
   */
  template<int dim, class GridImp>
  class OneDGridEntity<0,dim, GridImp> :
    public EntityDefault<0,dim,GridImp, OneDGridEntity>
  {
    friend class OneDGrid <dim, GridImp::dimensionworld>;
    friend class OneDGridIntersectionIterator <GridImp>;
    friend class OneDGridHierarchicIterator <GridImp>;
    friend class OneDGridLevelIterator <0,All_Partition,GridImp>;

    template <int cc_, int dim_, class GridImp_>
    friend class OneDGridSubEntityFactory;


  public:
    typedef typename GridImp::template codim<0>::Geometry Geometry;
    typedef typename GridImp::template codim<0>::LevelIterator LevelIterator;
    typedef typename GridImp::template codim<0>::IntersectionIterator IntersectionIterator;
    typedef typename GridImp::template codim<0>::HierarchicIterator HierarchicIterator;

    //! Constructor
    OneDGridEntity(int level) : level_(level), adaptationState(NONE), pred_(NULL), succ_(NULL) {
      sons_[0] = NULL;
      sons_[1] = NULL;
      father_ = NULL;
    }

    //! Destructor
    ~OneDGridEntity() {};

    //! Level of this element
    int level () const {return level_;}

    //! Index is unique and consecutive per level and codim
    int index () const {return index_;}

    /** \brief Return the global unique index in mesh
     * \todo So far returns the same as index()
     */
    int globalIndex() { return index(); }

    //! Geometry of this entity
    const Geometry& geometry () const {return geo_;}

    /** \brief Return the number of subentities of codimension cc.
     */
    template<int cc>
    int count () const {
      assert(cc==0 || cc==1);
      return (cc==0) ? 1 : 2;
    }

    /** \brief Return index of sub entity with codim = cc and local number i
     */
    template<int cc>
    int subIndex (int i) const {
      assert(i==0 || i==1);
      assert(cc==0 || cc==1);
      return entity<cc>(i)->index();
    }

    /** \brief Provide access to sub entity i of given codimension. Entities
     *  are numbered 0 ... count<cc>()-1
     */
    template<int cc>
    OneDGridLevelIterator<cc,All_Partition, GridImp> entity (int i) const;

    /*! Intra-level access to neighboring elements. A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor. */
    IntersectionIterator ibegin () const {
      return OneDGridIntersectionIterator<GridImp>(this);
    }

    //! Reference to one past the last neighbor
    IntersectionIterator iend () const {
      return IntersectionIterator(NULL);
    }

    //! returns true if Entity has no children
    bool isLeaf () const {
      return (sons_[0]==NULL) && (sons_[1]==NULL);
    }

    //! Inter-level access to father element on coarser grid.
    //! Assumes that meshes are nested.
    OneDGridLevelIterator<0,All_Partition, GridImp> father () {
      return OneDGridLevelIterator<0,All_Partition,GridImp>(father_);
    }

    /*! Location of this element relative to the reference element
       of the father. This is sufficient to interpolate all
       dofs in conforming case. Nonconforming may require access to
       neighbors of father and computations with local coordinates.
       On the fly case is somewhat inefficient since dofs  are visited
       several times. If we store interpolation matrices, this is tolerable.
       We assume that on-the-fly implementation of numerical algorithms
       is only done for simple discretizations. Assumes that meshes are nested.
       \todo Implement this!
     */
    OneDGridGeometry<dim, dim, GridImp>& father_relative_local () {
      DUNE_THROW(NotImplemented, "OneDGrid::father_relative_local() not implemented!");
    }

    /*! Inter-level access to son elements on higher levels<=maxlevel.
       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
     */
    HierarchicIterator hbegin (int maxlevel) {

      HierarchicIterator it(maxlevel);

      if (level()<maxlevel) {

        // Load sons of old target onto the iterator stack
        if (!isLeaf()) {
          typename HierarchicIterator::StackEntry se0;
          se0.element = sons_[0];
          se0.level   = level() + 1;
          it.elemStack.push(se0);

          typename HierarchicIterator::StackEntry se1;
          se1.element = sons_[1];
          se1.level   = level() + 1;
          it.elemStack.push(se1);
        }

      }

      it.target_ = (it.elemStack.empty()) ? NULL : it.elemStack.top().element;

      return it;
    }

    //! Returns iterator to one past the last son
    HierarchicIterator hend (int maxlevel) {
      return HierarchicIterator(maxlevel);;
    }

    // ***************************************************************
    //  Interface for Adaptation
    // ***************************************************************

    /** \todo Please doc me! */
    AdaptationState state() const {return adaptationState;}

  private:

    //! the current geometry
    OneDMakeableGeometry<dim,dim,GridImp> geo_;

    FixedArray<OneDGridEntity<0,dim,GridImp>*, 2> sons_;

    OneDGridEntity<0,dim,GridImp>* father_;

    //! element number
    int index_;

    //! the level of the entity
    int level_;

    OneDGridGeometry<dim,dim, GridImp>  fatherReLocal_;

    AdaptationState adaptationState;

  public:

    OneDGridEntity<0,dim,GridImp>* pred_;

    OneDGridEntity<0,dim,GridImp>* succ_;


  }; // end of OneDGridEntity codim = 0

} // namespace Dune

#endif
