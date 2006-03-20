// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRIDENTITY_HH
#define DUNE_UGGRIDENTITY_HH

/** \file
 * \brief The UGGridEntity class and its specializations
 */

#include "ugtypes.hh"
#include "uggridrenumberer.hh"

#include <dune/grid/common/referenceelements.hh>


namespace Dune {

  // Forward declarations
  template<int codim, int dim, class GridImp>
  class UGGridEntity;
  template<int dim, int dimworld>
  class UGGrid;
  template<int codim, class GridImp>
  class UGGridEntityPointer;
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
    public EntityDefaultImplementation <codim,dim,GridImp,UGGridEntity>
  {

    template <int codim_, PartitionIteratorType PiType_, class GridImp_>
    friend class UGGridLevelIterator;

    friend class UGMakeableEntity<codim,dim,GridImp>;

    friend class UGGrid<dim, dim>;

    template <class GridImp_>
    friend class UGGridLevelIndexSet;

    template <class GridImp_>
    friend class UGGridLeafIndexSet;

    typedef typename GridImp::ctype UGCtype;

  public:

    typedef typename GridImp::template Codim<codim>::Geometry Geometry;

    //! Constructor for an entity in a given grid level
    UGGridEntity(int level)
      : level_(level) {}

    //! level of this element
    int level () const {return level_;}

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
    const Geometry& geometry () const {return geo_;}

    UGGridEntityPointer<0,GridImp> ownersFather() const
    {
      UGGridEntityPointer<0,GridImp> myfather;
      if (UG_NS<dim>::NFather(target_)!=0)
        if (UG_NS<dim>::myLevel(UG_NS<dim>::NFather(target_)) == this->level()-1)
        {
          myfather.setToTarget(UG_NS<dim>::NFather(target_), this->level()-1);
          return myfather;
        }
      DUNE_THROW(NotImplemented, "ownersFather for anything else than new vertices");
      return myfather;
    }

    /** \brief Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
     *
       This can speed up on-the-fly interpolation for linear conforming elements
       Possibly this is sufficient for all applications we want on-the-fly.
     */
    const FieldVector<UGCtype, dim>& positionInOwnersFather() const
    {
      if (UG_NS<dim>::NFather(target_)!=0)
        if (UG_NS<dim>::myLevel(UG_NS<dim>::NFather(target_)) == this->level()-1)
        {
          UG_NS<dim>::PositionInFather(target_,pos_);
          return pos_;
        }
      DUNE_THROW(NotImplemented, "positionInOwnersFather for anything else than new vertices");
      return pos_;
    }



  private:


    void setToTarget(typename TargetType<codim,dim>::T* target) {
      target_ = target;
      geo_.setToTarget(target);
    }

    void setToTarget(typename TargetType<codim,dim>::T* target, int level) {
      target_ = target;
      level_  = level;
      geo_.setToTarget(target);
    }

    //! the current geometry
    UGMakeableGeometry<dim-codim,dim,GridImp> geo_;

    FieldVector<UGCtype, dim> localFatherCoords_;

    //! level
    int level_;

    typename TargetType<codim,dim>::T* target_;

    mutable FieldVector<UGCtype, dim> pos_;

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
    public EntityDefaultImplementation<0,dim,GridImp, UGGridEntity>
  {
    friend class UGGrid < dim , dim>;
    friend class UGGridIntersectionIterator <GridImp>;
    friend class UGGridHierarchicIterator <GridImp>;

    template <int codim_, PartitionIteratorType PiType_, class GridImp_>
    friend class UGGridLevelIterator;

    friend class UGMakeableEntity<0,dim,GridImp>;

    typedef typename GridImp::ctype UGCtype;

  public:

    typedef typename GridImp::template Codim<0>::Geometry Geometry;

    //! The Iterator over neighbors
    typedef UGGridIntersectionIterator<GridImp> IntersectionIterator;

    //! Iterator over descendants of the entity
    typedef UGGridHierarchicIterator<GridImp> HierarchicIterator;

    //! Constructor with a given grid level
    UGGridEntity(int level) : level_(level) {}

    //! Destructor
    ~UGGridEntity() {};

    //! Level of this element
    int level () const;

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

    /** \brief Return the number of subEntities of codimension cc.
     */
    template<int cc>
    int count () const;

    /** \brief Return level index of sub entity with codim = cc and local number i
     */
    template<int cc>
    int subIndex (int i) const {
      assert(i>=0 && i<count<cc>());

      if (cc==dim)
        return UG_NS<dim>::levelIndex(UG_NS<dim>::Corner(target_,UGGridRenumberer<dim>::verticesDUNEtoUG(i,geometry().type())));
      if (cc==0)
        return UG_NS<dim>::levelIndex(target_);
      if (cc==dim-1)
      {
        int a=ReferenceElements<double,dim>::general(geometry().type()).subEntity(i,dim-1,0,dim);
        int b=ReferenceElements<double,dim>::general(geometry().type()).subEntity(i,dim-1,1,dim);
        return UG_NS<dim>::levelIndex(UG_NS<dim>::GetEdge(UG_NS<dim>::Corner(target_,
                                                                             UGGridRenumberer<dim>::verticesDUNEtoUG(a,geometry().type())),
                                                          UG_NS<dim>::Corner(target_,
                                                                             UGGridRenumberer<dim>::verticesDUNEtoUG(b,geometry().type()))));
      }
      if (cc==1)
        return UG_NS<dim>::levelIndex(UG_NS<dim>::SideVector(target_,UGGridRenumberer<dim>::facesDUNEtoUG(i,geometry().type())));

      DUNE_THROW(GridError, "UGGrid<" << dim << "," << dim << ">::subIndex isn't implemented for cc==" << cc );
    }

    /** \brief Return leaf index of sub entity with codim = cc and local number i
     */
    template<int cc>
    int subLeafIndex (int i) const {
      assert(i>=0 && i<count<cc>());

      if (cc==dim)
        return UG_NS<dim>::leafIndex(UG_NS<dim>::Corner(target_,UGGridRenumberer<dim>::verticesDUNEtoUG(i,geometry().type())));
      if (cc==0)
        return UG_NS<dim>::leafIndex(target_);
      if (cc==dim-1)
      {
        int a=ReferenceElements<double,dim>::general(geometry().type()).subEntity(i,dim-1,0,dim);
        int b=ReferenceElements<double,dim>::general(geometry().type()).subEntity(i,dim-1,1,dim);
        return UG_NS<dim>::leafIndex(UG_NS<dim>::GetEdge(UG_NS<dim>::Corner(target_,
                                                                            UGGridRenumberer<dim>::verticesDUNEtoUG(a,geometry().type())),
                                                         UG_NS<dim>::Corner(target_,
                                                                            UGGridRenumberer<dim>::verticesDUNEtoUG(b,geometry().type()))));
      }
      if (cc==1)
        return UG_NS<dim>::leafIndex(UG_NS<dim>::SideVector(target_,UGGridRenumberer<dim>::facesDUNEtoUG(i,geometry().type())));

      DUNE_THROW(GridError, "UGGrid<" << dim << "," << dim << ">::subLeafIndex isn't implemented for cc==" << cc );
    }

    /** \brief Return global id of sub entity with codim = cc and local number i
     */
    template<int cc>
    unsigned int subGlobalId (int i) const {
      assert(i>=0 && i<count<cc>());

      if (cc==0)
      {
#ifdef ModelP
        return target_->ddd.gid;
#else
        return UG_NS<dim>::id(target_);
#endif
      }
      if (cc==dim)
      {
#ifdef ModelP
        return UG_NS<dim>::Corner(target_,UGGridRenumberer<dim>::verticesDUNEtoUG(i,geometry().type()))->ddd.gid;
#else
        return UG_NS<dim>::id(UG_NS<dim>::Corner(target_,UGGridRenumberer<dim>::verticesDUNEtoUG(i,geometry().type())));
#endif
      }

      DUNE_THROW(GridError, "UGGrid<" << dim << "," << dim << ">::subGlobalId isn't implemented for cc==" << cc );
      return 0;
    }

    /** \brief Return local id of sub entity with codim = cc and local number i
        \todo Only called from the id set
     */
    template<int cc>
    unsigned int subLocalId (int i) const {
      assert(i>=0 && i<count<cc>());

      if (cc==dim)
        return UG_NS<dim>::id(UG_NS<dim>::Corner(target_,UGGridRenumberer<dim>::verticesDUNEtoUG(i,geometry().type())));
      else if (cc==0)
        return UG_NS<dim>::id(target_);
      else
        DUNE_THROW(GridError, "UGGrid<" << dim << "," << dim << ">::subLocalId isn't implemented for cc==" << cc );
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
    UGGridIntersectionIterator<GridImp> ibegin () const;

    //! Reference to one past the last neighbor
    UGGridIntersectionIterator<GridImp> iend () const;

    //! returns true if Entity has NO children
    bool isLeaf() const {
      return UG_NS<dim>::isLeaf(target_);
    }

    //! returns true if element is of regular type
    bool isRegular() const {
      return UG_NS<dim>::isRegular(target_);
    }

    //! Inter-level access to father element on coarser grid.
    //! Assumes that meshes are nested.
    UGGridLevelIterator<0,All_Partition,GridImp> father () const;

    /*! Location of this element relative to the reference element element of the father.
       This is sufficient to interpolate all dofs in conforming case.
       Nonconforming may require access to neighbors of father and
       computations with local coordinates.
       On the fly case is somewhat inefficient since dofs  are visited several times.
       If we store interpolation matrices, this is tolerable. We assume that on-the-fly
       implementation of numerical algorithms is only done for simple discretizations.
       Assumes that meshes are nested.
     */
    const Geometry& geometryInFather () const;

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

    //! geometry for mapping into father's reference element
    mutable UGMakeableGeometry<dim,GridImp::dimensionworld,GridImp> geometryInFather_;

    //! the level of the entity
    int level_;

    typename TargetType<0,dim>::T* target_;


  }; // end of UGGridEntity codim = 0

} // namespace Dune

#endif
