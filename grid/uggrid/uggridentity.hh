// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRIDENTITY_HH
#define DUNE_UGGRIDENTITY_HH

/** \file
 * \brief The UGGridEntity class and its specializations
 */

#include "ugtypes.hh"


namespace Dune {

  // Forward declarations
  template<int codim, int dim, class GridImp>
  class UGGridEntity;
  template<int dim, int dimworld>
  class UGGrid;
  template<int codim, PartitionIteratorType pitype, class GridImp>
  class UGGridLevelIterator;
  template<class GridImp>
  class UGGridIntersectionIterator;
  template<class GridImp>
  class UGGridHierarchicIterator;

  template<int codim, int dim, class GridImp>
  class UGMakeableEntity :
    public GridImp::template Codim<codim>::Entity
  {
  public:

    UGMakeableEntity(int level) :
      GridImp::template Codim<codim>::Entity (UGGridEntity<codim, dim, const GridImp>(level))
    {}

    void setToTarget(typename TargetType<codim,dim>::T* target) {
      this->realEntity.setToTarget(target);
    }

    void setToTarget(typename TargetType<codim,dim>::T* target, int level) {
      this->realEntity.setToTarget(target, level);
    }

    typename TargetType<codim,dim>::T* getTarget() {
      return this->realEntity.target_;
    }

  };

  //**********************************************************************
  //
  // --UGGridEntity
  // --Entity
  //
  /** \brief The implementation of entities in a UGGrid
     \ingroup UGGrid

     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

   */
  template<int codim, int dim, class GridImp>
  class UGGridEntity :
    public EntityDefault <codim,dim,GridImp,UGGridEntity>
  {

    template <int codim_, PartitionIteratorType PiType_, class GridImp_>
    friend class UGGridLevelIterator;

    friend class UGMakeableEntity<codim,dim,GridImp>;

    friend class UGGrid<dim, dim>;

    template <int dim_>
    friend class UGGridLevelIndexSet;
    template <int dim_>
    friend class UGGridLeafIndexSet;

    typedef typename GridImp::ctype UGCtype;

  public:

    typedef typename GridImp::template Codim<codim>::Geometry Geometry;

    //! Constructor for an entity in a given grid level
    UGGridEntity(int level);

    //! level of this element
    int level () const;

    //! index is unique and consecutive per level and codim
    //! used for access to degrees of freedom
    int index () const {
      return levelIndex();
    }

    int levelIndex() const {
      return UG_NS<dim>::levelIndex(target_);
    }

    int leafIndex() const {
      return target_->myvertex->iv.id;
    }

    unsigned int localId() const {
      return target_->id;
    }

    unsigned int globalId() const {
#ifdef ModelP
      return target_->ddd.gid;
#else
      return target_->id;
#endif
    }


    /** \brief The partition type for parallel computing
     * \todo So far it always returns InteriorEntity */
    PartitionType partitionType () const { return InteriorEntity; }

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
       with codimension cc.
     */
    //!< Default codim 1 Faces and codim == dim Vertices
    template<int cc> int count () const;

    //! return index of sub entity with codim = cc and local number i
    //! i.e. return global number of vertex i
    /** \todo So far only implemented for cc==dim */
    template<int cc> int subIndex (int i) const;

    //! geometry of this entity
    const Geometry& geometry () const;

    UGGridLevelIterator<0,All_Partition,GridImp> ownersFather() const {
      DUNE_THROW(NotImplemented, "ownersFather");
    }

    /** \brief Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
     *
       This can speed up on-the-fly interpolation for linear conforming elements
       Possibly this is sufficient for all applications we want on-the-fly.
     */
    FieldVector<UGCtype, dim>& positionInOwnersFather() const {
      DUNE_THROW(NotImplemented, "positionInOwnersFather");
    }



  private:


    void setToTarget(typename TargetType<codim,dim>::T* target);

    void setToTarget(typename TargetType<codim,dim>::T* target, int level);

    //! the current geometry
    UGMakeableGeometry<dim-codim,dim,GridImp> geo_;

    FieldVector<UGCtype, dim> localFatherCoords_;

    //! level
    int level_;

    typename TargetType<codim,dim>::T* target_;
  };

  //***********************
  //
  //  --UGGridEntity
  //  --0Entity
  //
  //***********************

  /** \brief Specialization for codim-0-entities.
   * \ingroup UGGrid
   *
   * This class embodies the topological parts of elements of the grid.
   * It has an extended interface compared to the general entity class.
   * For example, Entities of codimension 0  allow to visit all neighbors.
   *
   * UGGrid only implements the cases dim==dimworld==2 and dim=dimworld==3.
   */
  template<int dim, class GridImp>
  class UGGridEntity<0,dim,GridImp> :
    public EntityDefault<0,dim,GridImp, UGGridEntity>
  {
    friend class UGGrid < dim , dim>;
    friend class UGGridIntersectionIterator <GridImp>;
    friend class UGGridHierarchicIterator <GridImp>;

    template <int codim_, PartitionIteratorType PiType_, class GridImp_>
    friend class UGGridLevelIterator;

    friend class UGMakeableEntity<0,dim,GridImp>;

    // Either UG3d::ELEMENT or UG2d:ELEMENT
    typedef typename TargetType<0,dim>::T UGElementType;

  public:

    typedef typename GridImp::template Codim<0>::Geometry Geometry;

    //! The Iterator over neighbors
    typedef UGGridIntersectionIterator<GridImp> IntersectionIterator;

    //! Iterator over descendants of the entity
    typedef UGGridHierarchicIterator<GridImp> HierarchicIterator;

    //! Constructor with a given grid level
    UGGridEntity(int level);

    //! Destructor
    ~UGGridEntity() {};

    //! Level of this element
    int level () const;

    //! Index is unique and consecutive per level and codim
    int index () const {
      return levelIndex();
    }

    int levelIndex() const {
      return UG_NS<dim>::levelIndex(target_);
    }

    int leafIndex() const {
      return UG_NS<dim>::leafIndex(target_);
    }

    unsigned int localId() const {
      return UG_NS<dim>::id(target_);
    }

    unsigned int globalId() const {
#ifdef ModelP
      return target_->ddd.gid;
#else
      return UG_NS<dim>::id(target_);
#endif
    }

    /** \brief The partition type for parallel computing */
    PartitionType partitionType () const {
#ifndef ModelP
      return InteriorEntity;
#else
#define PARHDRE(p) (&((p)->ge.ddd))
#define EPRIO(e) DDD_InfoPriority(PARHDRE(e))
      if (EPRIO(target_) == UG::PrioHGhost
          || EPRIO(target_) == UG::PrioVGhost
          || EPRIO(target_) == UG::PrioVHGhost)
        return GhostEntity;
      else
        return InteriorEntity;
#undef EPRIO
#undef PARHDRE
#endif
    }

    //! Geometry of this entity
    const Geometry& geometry () const;

    /** \brief Return the number of subentities of codimension cc.
     */
    template<int cc>
    int count () const;

    /** \brief Return level index of sub entity with codim = cc and local number i
     */
    template<int cc>
    int subIndex (int i) const;

    /** \brief Return leaf index of sub entity with codim = cc and local number i
     */
    template<int cc>
    int subLeafIndex (int i) const;

    /** \brief Return global id of sub entity with codim = cc and local number i
     */
    template<int cc>
    int subGlobalId (int i) const;

    /** \brief Return local id of sub entity with codim = cc and local number i
     */
    template<int cc>
    int subLocalId (int i) const;


    /** \brief Provide access to sub entity i of given codimension. Entities
     *  are numbered 0 ... count<cc>()-1
     */
    template<int cc>
    typename GridImp::template Codim<cc>::EntityPointer entity (int i) const;

    /*! Intra-level access to neighboring elements. A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor. */
    UGGridIntersectionIterator<GridImp> ibegin () const;

    //! Reference to one past the last neighbor
    UGGridIntersectionIterator<GridImp> iend () const;

    //! returns true if Entity has children
    bool isLeaf() const {
      return UG_NS<dim>::isLeaf(target_);
    }

    //! Inter-level access to father element on coarser grid.
    //! Assumes that meshes are nested.
    UGGridLevelIterator<0,All_Partition,GridImp> father () const;

    /*! Location of this element relative to the reference element
       of the father. This is sufficient to interpolate all
       dofs in conforming case. Nonconforming may require access to
       neighbors of father and computations with local coordinates.
       On the fly case is somewhat inefficient since dofs  are visited
       several times. If we store interpolation matrices, this is tolerable.
       We assume that on-the-fly implementation of numerical algorithms
       is only done for simple discretizations. Assumes that meshes are nested.
     */
    UGGridGeometry<dim,dim,GridImp>& father_relative_local();

    /*! Inter-level access to son elements on higher levels<=maxlevel.
       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
     */
    UGGridHierarchicIterator<GridImp> hbegin (int maxlevel) const;

    //! Returns iterator to one past the last son
    UGGridHierarchicIterator<GridImp> hend (int maxlevel) const;

    //***************************************************************
    //  Interface for Adaptation
    //***************************************************************

    /** \todo Please doc me! */
    AdaptationState state() const;

    //private:

    void setToTarget(typename TargetType<0,dim>::T* target, int level);

    //! Leaves the level untouched
    void setToTarget(typename TargetType<0,dim>::T* target);

    //! the current geometry
    UGMakeableGeometry<dim,GridImp::dimensionworld,GridImp> geo_;

    //! the level of the entity
    int level_;

    typename TargetType<0,dim>::T* target_;

    UGGridGeometry <dim,dim,GridImp> fatherReLocal_;

  }; // end of UGGridEntity codim = 0

  // Include class method definitions
#include "uggridentity.cc"

} // namespace Dune

#endif
