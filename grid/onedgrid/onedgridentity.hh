// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONE_D_GRID_ENTITY_HH
#define DUNE_ONE_D_GRID_ENTITY_HH

#include <dune/common/fixedarray.hh>


/** \file
 * \brief The OneDGridEntity class and its specializations
 */

namespace Dune {

  template <int mydim>
  class OneDEntityImp {};

  template <>
  class OneDEntityImp<0>
  {
  public:

    OneDEntityImp(int level, double pos) : pos_(pos), level_(level), son_(NULL), pred_(NULL), succ_(NULL)
    {}

    OneDEntityImp(int level, const FieldVector<double, 1>& pos, unsigned int id)
      : pos_(pos), id_(id), level_(level), son_(NULL), pred_(NULL), succ_(NULL)
    {}
    //private:
    bool isLeaf() const {
      return son_==NULL;
    }

    FieldVector<double, 1> pos_;

    //! entity number
    unsigned int levelIndex_;

    unsigned int leafIndex_;

    unsigned int id_;

    //! level
    int level_;

    //! Son vertex on the next finer grid
    OneDEntityImp<0>* son_;

    //!
    OneDEntityImp<0>* pred_;

    OneDEntityImp<0>* succ_;


  };


  template <>
  class OneDEntityImp<1>
  {
  public:

    OneDEntityImp(int level, unsigned int id)
      : id_(id), level_(level),
        markState_(NONE), adaptationState_(NONE),
        pred_(NULL), succ_(NULL)
    {
      sons_[0] = sons_[1] = NULL;
    }

    bool isLeaf() const {
      return sons_[0]==NULL && sons_[1]==NULL;
    }

    FixedArray<OneDEntityImp<1>*, 2> sons_;

    OneDEntityImp<1>* father_;

    OneDEntityImp<0>* vertex_[2];

    //! element number
    unsigned int levelIndex_;

    unsigned int leafIndex_;

    /** \brief Unique and persistent id for elements */
    unsigned int id_;

    //! the level of the entity
    int level_;

    //OneDGridGeometry<dim,dim, GridImp>  fatherReLocal_;

    /** \brief Stores requests for refinement and coarsening */
    AdaptationState markState_;

    /** \brief Stores information about prospective refinement and coarsening
        for use in the interface method state() */
    AdaptationState adaptationState_;

    /** \brief Predecessor in the doubly linked list of elements */
    OneDEntityImp<1>* pred_;

    /** \brief Successor in the doubly linked list of elements */
    OneDEntityImp<1>* succ_;

  };

}

#include "onedgridgeometry.hh"

namespace Dune {

  template<int cd, int dim, class GridImp>
  class OneDEntityWrapper :
    public GridImp::template Codim<cd>::Entity
  {
  public:

    OneDEntityWrapper() :
      GridImp::template Codim<cd>::Entity (OneDGridEntity<cd, dim, GridImp>())
    {}

    void setToTarget(OneDEntityImp<dim-cd>* target) {
      this->realEntity.setToTarget(target);
    }

    OneDEntityImp<dim-cd>* target() {return this->realEntity.target_;}
  };


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
  template<int cd, int dim, class GridImp>
  class OneDGridEntity :
    public EntityDefault <cd,dim,GridImp,OneDGridEntity>
  {

    template <int codim_, PartitionIteratorType PiType_, class GridImp_>
    friend class OneDGridLevelIterator;

    friend class OneDGrid<dim,GridImp::dimensionworld>;

    //! Constructor with a given grid level
    OneDGridEntity(int level, double coord) : geo_(coord), target_(NULL) {}

  public:
    //! Constructor with a given grid level
    OneDGridEntity() : target_(NULL) {}

    typedef typename GridImp::template Codim<cd>::Geometry Geometry;
    typedef typename GridImp::template Codim<cd>::LevelIterator LevelIterator;
    typedef typename GridImp::template Codim<0>::IntersectionIterator IntersectionIterator;
    typedef typename GridImp::template Codim<0>::HierarchicIterator HierarchicIterator;

    //! level of this element
    int level () const {return target_->level_;}

    unsigned int levelIndex() const {return target_->levelIndex_;}

    unsigned int leafIndex() const {return target_->leafIndex_;}

    unsigned int globalId() const {return target_->id_;}

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
       with codimension cc.
     */
    //!< Default codim 1 Faces and codim == dim Vertices
    template<int cc> int count () const;

    //! Provide access to mesh entity i of given codimension. Entities
    //!  are numbered 0 ... count<cc>()-1
    template<int cc>
    OneDGridLevelIterator<cc,All_Partition, GridImp> entity (int i);

    //! geometry of this entity
    const Geometry& geometry () const {return geo_;}

    OneDGridLevelIterator<0,All_Partition, GridImp> ownersFather () const {
      DUNE_THROW(NotImplemented, "ownersFather");
      //return OneDGridLevelIterator<cd,All_Partition,GridImp>();
    }

    /** \brief Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
     *
       This can speed up on-the-fly interpolation for linear conforming elements
       Possibly this is sufficient for all applications we want on-the-fly.
     */
    FieldVector<OneDCType, dim>& positionInOwnersFather () const {
      DUNE_THROW(NotImplemented, "positionInOwnersFather");
    }

    void setToTarget(OneDEntityImp<0>* target) {
      target_ = target;
      geo_.setToTarget(target);
    }

    //! the current geometry
    OneDMakeableGeometry<dim-cd,dim,GridImp> geo_;

    OneDEntityImp<0>* target_;

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
    typedef typename GridImp::template Codim<0>::Geometry Geometry;
    typedef typename GridImp::template Codim<0>::LevelIterator LevelIterator;
    typedef typename GridImp::template Codim<0>::IntersectionIterator IntersectionIterator;
    typedef typename GridImp::template Codim<0>::HierarchicIterator HierarchicIterator;

    //! Default Constructor
    OneDGridEntity() {};


    //! Level of this element
    int level () const {return target_->level_;}

    //! Level index is unique and consecutive per level and codim
    unsigned int levelIndex() const {return target_->levelIndex_;}

    unsigned int leafIndex() const {return target_->leafIndex_;}

    unsigned int globalId() const {return target_->id_;}

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
    int subLevelIndex (int i) const {
      assert(i==0 || i==1);
      return (cc==0)
             ? target_->levelIndex_
             : target_->vertex_[i]->levelIndex_;
    }

    /** \brief Return leaf index of sub entity with codim = cc and local number i
     */
    template<int cc>
    int subLeafIndex (int i) const {
      assert(i==0 || i==1);
      return (cc==0)
             ? target_->leafIndex_
             : target_->vertex_[i]->leafIndex_;
    }

    /** \brief Return leaf index of sub entity with codim = cc and local number i
     */
    template<int cc>
    int subId (int i) const {
      assert(i==0 || i==1);
      return (cc==0)
             ? target_->id_
             : target_->vertex_[i]->id_;
    }

    /** \brief Provide access to sub entity i of given codimension. Entities
     *  are numbered 0 ... count<cc>()-1
     */
    template<int cc>
    typename GridImp::template Codim<cc>::EntityPointer entity (int i) const;

    /*! Intra-level access to neighboring elements. A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor. */
    IntersectionIterator ibegin () const {
      return OneDGridIntersectionIterator<GridImp>(target_, 0);
    }

    //! Reference to one past the last neighbor
    IntersectionIterator iend () const {
      return OneDGridIntersectionIterator<GridImp>(target_, 2);
    }

    //! returns true if Entity has no children
    bool isLeaf () const {
      return (target_->sons_[0]==NULL) && (target_->sons_[1]==NULL);
    }

    //! Inter-level access to father element on coarser grid.
    //! Assumes that meshes are nested.
    OneDGridEntityPointer<0, GridImp> father () const {
      return OneDGridEntityPointer<0,GridImp>(target_->father_);
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
    const Geometry& geometryInFather () const {
      DUNE_THROW(NotImplemented, "OneDGrid::geometryInFather() not implemented!");

    }

    /*! Inter-level access to son elements on higher levels<=maxlevel.
       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
     */
    OneDGridHierarchicIterator<GridImp> hbegin (int maxlevel) const {

      OneDGridHierarchicIterator<GridImp> it(maxlevel);

      if (level()<=maxlevel) {

        // Load sons of old target onto the iterator stack
        if (!isLeaf()) {
          typename OneDGridHierarchicIterator<GridImp>::StackEntry se0;
          se0.element = target_->sons_[0];
          se0.level   = level() + 1;
          it.elemStack.push(se0);

          typename OneDGridHierarchicIterator<GridImp>::StackEntry se1;
          se1.element = target_->sons_[1];
          se1.level   = level() + 1;
          it.elemStack.push(se1);
        }

      }

      it.virtualEntity_.setToTarget((it.elemStack.empty()) ? NULL : it.elemStack.top().element);

      return it;
    }

    //! Returns iterator to one past the last son
    HierarchicIterator hend (int maxlevel) const {
      return HierarchicIterator(maxlevel);
    }

    // ***************************************************************
    //  Interface for Adaptation
    // ***************************************************************

    /** \todo Please doc me! */
    AdaptationState state() const {return target_->adaptationState_;}

    void setToTarget(OneDEntityImp<1>* target) {
      target_ = target;
      geo_.setToTarget(target);
    }


    //! the current geometry
    OneDMakeableGeometry<dim,dim,GridImp> geo_;

    OneDEntityImp<1>* target_;

  }; // end of OneDGridEntity codim = 0

} // namespace Dune

#include "onedgridentity.cc"

#endif
