// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_UGGRIDENTITY_HH__
#define __DUNE_UGGRIDENTITY_HH__

#include "ugtypes.hh"


namespace Dune {

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
  template<int codim, int dim, int dimworld>
  class UGGridEntity :
    public EntityDefault <codim,dim,dimworld, UGCtype,
        UGGridEntity,UGGridElement,UGGridLevelIterator,
        UGGridIntersectionIterator,UGGridHierarchicIterator>
  {

    template <int codim_, int dim_, int dimworld_, PartitionIteratorType PiType_>
    friend class UGGridLevelIterator;

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
    template<int cc> UGGridLevelIterator<cc,dim,dimworld,All_Partition> entity (int i);

    //! Constructor for an entity in a given grid level
    UGGridEntity(int level);

    //! geometry of this entity
    UGGridElement<dim-codim,dimworld>& geometry ();

    /** \brief Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
     *
       This can speed up on-the-fly interpolation for linear conforming elements
       Possibly this is sufficient for all applications we want on-the-fly.
     */
    UGGridLevelIterator<0,dim,dimworld,All_Partition> father ();

    //! local coordinates within father
    FieldVector<UGCtype, dim>& local ();



  private:


    void setToTarget(typename TargetType<codim,dim>::T* target);

    void setToTarget(typename TargetType<codim,dim>::T* target, int level);

    // private Methods
    void makeDescription();

    //! the current geometry
    UGGridElement<dim-codim,dimworld> geo_;
    bool builtgeometry_;       //!< true if geometry has been constructed

    FieldVector<UGCtype, dim> localFatherCoords_;

    //! element number
    int elNum_;

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
  template<int dim, int dimworld>
  class UGGridEntity<0,dim,dimworld> :
    public EntityDefault<0,dim,dimworld, UGCtype,UGGridEntity,UGGridElement,
        UGGridLevelIterator,UGGridIntersectionIterator,
        UGGridHierarchicIterator>
  {
    friend class UGGrid < dim , dimworld >;
    friend class UGGridIntersectionIterator < dim, dimworld>;
    friend class UGGridHierarchicIterator < dim, dimworld>;
    friend class UGGridLevelIterator <0,dim,dimworld,All_Partition>;

    // Either UG3d::ELEMENT or UG2d:ELEMENT
    typedef typename TargetType<0,dim>::T UGElementType;

  public:

    //! The Iterator over neighbors
    typedef UGGridIntersectionIterator<dim,dimworld> IntersectionIterator;

    //! Iterator over descendants of the entity
    typedef UGGridHierarchicIterator<dim,dimworld> HierarchicIterator;

    //! Constructor with a given grid level
    UGGridEntity(int level);

    //! Destructor
    ~UGGridEntity() {};

    //! Level of this element
    int level ();

    //! Index is unique and consecutive per level and codim
    int index ();

    /** \brief Return the global unique index in mesh
     * \todo So far returns the same as index()
     */
    int globalIndex() { return index(); }

    //! Geometry of this entity
    UGGridElement<dim,dimworld>& geometry ();

    /** \brief Return the number of subentities of codimension cc.
     */
    template<int cc> int count ();

    /** \brief Return index of sub entity with codim = cc and local number i
     */
    template<int cc> int subIndex (int i);

    /** \brief Provide access to sub entity i of given codimension. Entities
     *  are numbered 0 ... count<cc>()-1
     */
    template<int cc>
    UGGridLevelIterator<cc,dim,dimworld, All_Partition> entity (int i);

    /*! Intra-level access to neighboring elements. A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor. */
    UGGridIntersectionIterator<dim,dimworld> ibegin ();

    //! Reference to one past the last neighbor
    UGGridIntersectionIterator<dim,dimworld> iend ();

    //! returns true if Entity has children
    bool hasChildren ();

    //! Inter-level access to father element on coarser grid.
    //! Assumes that meshes are nested.
    UGGridLevelIterator<0,dim,dimworld,All_Partition> father ();

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

    void setToTarget(typename TargetType<0,dim>::T* target, int level);

    //! Leaves the level untouched
    void setToTarget(typename TargetType<0,dim>::T* target);

    //! the current geometry
    UGGridElement<dim,dimworld> geo_;
    bool builtgeometry_; //!< true if geometry has been constructed

    //! element number
    int elNum_;

    //! the level of the entity
    int level_;

    typename TargetType<0,dim>::T* target_;

    UGGridElement <dim,dim> fatherReLocal_;

  }; // end of UGGridEntity codim = 0

  // Include class method definitions
#include "uggridentity.cc"

} // namespace Dune

#endif
