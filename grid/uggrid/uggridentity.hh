// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRIDENTITY_HH
#define DUNE_UGGRIDENTITY_HH

#include "ugtypes.hh"


namespace Dune {

  //**********************************************************************
  //
  // --UGGridEntity
  // --Entity
  //
  /** \brief The implementation of entities in a UGGrid

     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

   */
  template<int codim, int dim, int dimworld>
  class UGGridEntity :
    public EntityDefault <codim,dim,dimworld, UGCtype,
        UGGridEntity,UGGridElement,UGGridLevelIterator,
        UGGridIntersectionIterator,UGGridHierarchicIterator>
  {
    //friend class UGGrid < dim , dimworld >;
    //friend class UGGridEntity < 0, dim, dimworld>;
    friend class UGGridLevelIterator < codim, dim, dimworld>;
    //friend class UGGridLevelIterator < dim, dim, dimworld>;
    //friend class UGGridIntersectionIterator < dim, dimworld>;
  public:

    //! level of this element
    int level ();

    //! index is unique and consecutive per level and codim
    //! used for access to degrees of freedom
    int index ();

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
       with codimension cc.
     */
    //!< Default codim 1 Faces and codim == dim Vertices
    template<int cc> int count ();

    //! return index of sub entity with codim = cc and local number i
    //! i.e. return global number of vertex i
    /** \todo So far only implemented for cc==dim */
    template<int cc> int subIndex (int i);

    //! Provide access to mesh entity i of given codimension. Entities
    //!  are numbered 0 ... count<cc>()-1
    template<int cc> UGGridLevelIterator<cc,dim,dimworld> entity (int i);

    UGGridEntity(int level);

    /*! Intra-level access to intersection with neighboring elements.
       A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor. */
    UGGridIntersectionIterator<dim,dimworld> ibegin ();

    //! same method for fast access
    //void ibegin (AlbertGridIntersectionIterator<dim,dimworld> &it);

    //! Reference to one past the last intersection with neighbor
    UGGridIntersectionIterator<dim,dimworld> iend ();

    //! geometry of this entity
    UGGridElement<dim-codim,dimworld>& geometry ();

    /*! Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
       This can speed up on-the-fly interpolation for linear conforming elements
       Possibly this is sufficient for all applications we want on-the-fly.
     */
    UGGridLevelIterator<0,dim,dimworld> father ();

    //! local coordinates within father
    Vec<dim, UGCtype>& local ();



  private:


    void setToTarget(typename TargetType<codim,dim>::T* target);

    // private Methods
    void makeDescription();

    //UGGrid<dim,dimworld> &grid_;

    //! the current geometry
    UGGridElement<dim-codim,dimworld> geo_;
    bool builtgeometry_;       //!< true if geometry has been constructed

    Vec<dim,UGCtype> localFatherCoords_;

    //! element number
    int elNum_;

    //! level
    int level_;

    typename TargetType<codim,dim>::T* target_;
  };

  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Entities of codimension 0 ("elements") are defined through template specialization. Note
     that this specialization has an extended interface compared to the general case

     Entities of codimension 0  allow to visit all neighbors, where
     a neighbor is an entity of codimension 0 which has a common entity of codimension 1 with the
     These neighbors are accessed via an iterator. This allows the implementation of
     non-matching meshes. The number of neigbors may be different from the number of faces/edges
     of an element!
   */
  //***********************
  //
  //  --UGGridEntity
  //  --0Entity
  //
  //***********************
  template<int dim, int dimworld>
  class UGGridEntity<0,dim,dimworld> :
    public EntityDefault<0,dim,dimworld, UGCtype,UGGridEntity,UGGridElement,
        UGGridLevelIterator,UGGridIntersectionIterator,
        UGGridHierarchicIterator>
  {
    friend class UGGrid < dim , dimworld >;
    friend class UGMarkerVector;
    friend class UGGridIntersectionIterator < dim, dimworld>;
    friend class UGGridHierarchicIterator < dim, dimworld>;
    friend class UGGridLevelIterator <0,dim,dimworld>;
  public:
    typedef UGGridIntersectionIterator<dim,dimworld> IntersectionIterator;
    typedef UGGridHierarchicIterator<dim,dimworld> HierarchicIterator;

    //! there are only implementations for dim==dimworld 2,3
    ~UGGridEntity() {};

    UGGridEntity(int level);

    //! Constructor, real information is set via setElInfo method
    //UGGridEntity(UGGrid<dim,dimworld> &grid, int level);

    //! level of this element
    int level ();

    //! index is unique and consecutive per level and codim used for access to degrees of freedo
    int index ();

    //! geometry of this entity
    UGGridElement<dim,dimworld>& geometry ();

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
       with codimension cc.
     */
    //!< Default codim 1 Faces and codim == dim Vertices
    template<int cc> int count ();

    //! return index of sub entity with codim = cc and local number i
    //! i.e. return global number of vertex i
    template<int cc> int subIndex (int i);

    //! Provide access to mesh entity i of given codimension. Entities
    //!  are numbered 0 ... count<cc>()-1
    template<int cc> UGGridLevelIterator<cc,dim,dimworld> entity (int i);

    /*! Intra-level access to neighboring elements. A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor. */
    UGGridIntersectionIterator<dim,dimworld> nbegin ();

    //! Reference to one past the last neighbor
    UGGridIntersectionIterator<dim,dimworld> nend ();

    //! returns true if Entity has children
    bool hasChildren ();

    //! return number of layers far from refined elements of this level
    int refDistance ();

    //! Inter-level access to father element on coarser grid.
    //! Assumes that meshes are nested.
    UGGridLevelIterator<0,dim,dimworld> father ();

    /*! Location of this element relative to the reference element
       of the father. This is sufficient to interpolate all
       dofs in conforming case. Nonconforming may require access to
       neighbors of father and computations with local coordinates.
       On the fly case is somewhat inefficient since dofs  are visited
       several times. If we store interpolation matrices, this is tolerable.
       We assume that on-the-fly implementation of numerical algorithms
       is only done for simple discretizations. Assumes that meshes are nested.
     */
    UGGridElement<dim,dim>& father_relative_local ();

    /*! Inter-level access to son elements on higher levels<=maxlevel.
       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
     */
    UGGridHierarchicIterator<dim,dimworld> hbegin (int maxlevel);

    //! Returns iterator to one past the last son
    UGGridHierarchicIterator<dim,dimworld> hend (int maxlevel);

    //***************************************************************
    //  Interface for Adaptation
    //***************************************************************

    /** \brief Mark entity for refinement
     *
     * This only works for entities of codim 0.
     * The parameter is currently ignored
     *
     * \return <ul>
     * <li> true, if element was marked </li>
     * <li> false, if nothing changed </li>
     * </ul>
     */
    bool mark(int refCount);

    /** \todo Please doc me! */
    AdaptationState state() const;

  private:
    // called from HierarchicIterator, because only this
    // class changes the level of the entity, otherwise level is set by
    // Constructor
    void setLevel ( int actLevel );

    // return the global unique index in mesh
    int globalIndex() { return elInfo_->el->index; }

    //! make a new UGGridEntity
    void makeDescription();

    void setToTarget(typename TargetType<0,dim>::T* target);

    //! the corresponding grid
    //UGGrid<dim,dimworld> &grid_;

    //! for vertex access, to be revised, filled on demand
    //UGGridLevelIterator<dim,dim,dimworld> vxEntity_;

    //! the cuurent geometry
    UGGridElement<dim,dimworld> geo_;
    bool builtgeometry_; //!< true if geometry has been constructed

    //! element number
    int elNum_;

    //! the level of the entity
    int level_;

    typename TargetType<0,dim>::T* target_;
    //UGGridElement <dim,dim> fatherReLocal_;
  }; // end of UGGridEntity codim = 0


} // namespace Dune

#endif
