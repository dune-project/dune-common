// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRIDENTITY_HH
#define DUNE_UGGRIDENTITY_HH



//**********************************************************************
//
// --UGGridEntity
// --Entity
//
/*!
   A Grid is a container of grid entities. An entity is parametrized by the codimension.
   An entity of codimension c in dimension d is a d-c dimensional object.

   Here: the general template
 */
template<int codim, int dim, int dimworld>
class UGGridEntity :
  public EntityDefault <codim,dim,dimworld, UGCtype,
      UGGridEntity,UGGridElement,UGGridLevelIterator,
      UGGridIntersectionIterator,UGGridHierarchicIterator>
{
  friend class UGGrid < dim , dimworld >;
  //friend class UGGridEntity < 0, dim, dimworld>;
  friend class UGGridLevelIterator < codim, dim, dimworld>;
  //friend class UGGridLevelIterator < dim, dim, dimworld>;
  friend class UGGridIntersectionIterator < dim, dimworld>;
public:
  //! know your own codimension
  //enum { codimension=codim };

  //! know your own dimension
  //enum { dimension=dim };

  //! know your own dimension of world
  //enum { dimensionworld=dimworld };

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
  /* template<int cc> */ int subIndex (int i);

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


  void setToTarget(TargetType<codim,dim>::T* target);

  // returns the global vertex number as default
  //int globalIndex() { return elInfo_->el->dof[vertex_][0]; }

  // private Methods
  void makeDescription();

  //UGGrid<dim,dimworld> &grid_;

  //! the current geometry
  UGGridElement<dim-codim,dimworld> geo_;
  bool builtgeometry_;         //!< true if geometry has been constructed

  Vec<dim,UGCtype> localFatherCoords_;

  //! element number
  int elNum_;

  //! level
  int level_;

  TargetType<codim,dim>::T* target_;
};

#if 0
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

  //! know your own codimension
  //enum { codimension=0 };

  //! know your own dimension
  //enum { dimension=dim };

  //! know your own dimension of world
  //enum { dimensionworld=dimworld };

  //! Destructor, needed perhaps needed for deleteing faceEntity_ and
  //! edgeEntity_ , see below
  //! there are only implementations for dim==dimworld 2,3
  ~UGGridEntity() {};

  //! Constructor, real information is set via setElInfo method
  UGGridEntity(UGGrid<dim,dimworld> &grid, int level);

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

  //! marks an element for refCount refines. if refCount is negative the
  //! element is coarsend -refCount times
  //! mark returns true if element was marked, otherwise false
  bool mark( int refCount );

private:
  // called from HierarchicIterator, because only this
  // class changes the level of the entity, otherwise level is set by
  // Constructor
  void setLevel ( int actLevel );

#if 0
  // face, edge and vertex only for codim > 0, in this
  // case just to supply the same interface
  void setTraverseStack (ALBERT TRAVERSE_STACK *travStack);
  void setElInfo (ALBERT EL_INFO *elInfo,
                  int elNum = 0,
                  unsigned char face = 0,
                  unsigned char edge = 0,
                  unsigned char vertex = 0 );

  // needed for LevelIterator to compare
  ALBERT EL_INFO *getElInfo () const;
#endif

  // return the global unique index in mesh
  int globalIndex() { return elInfo_->el->index; }

  //! make a new UGGridEntity
  void makeDescription();

  //! the corresponding grid
  UGGrid<dim,dimworld> &grid_;

  //! for vertex access, to be revised, filled on demand
  UGGridLevelIterator<dim,dim,dimworld> vxEntity_;

  //! the cuurent geometry
  UGGridElement<dim,dimworld> geo_;
  bool builtgeometry_;  //!< true if geometry has been constructed

#if 0
  //! pointer to the real UG element data
  ALBERT EL_INFO *elInfo_;
#endif

  //! the level of the entity
  int level_;


  UGGridElement <dim,dim> fatherReLocal_;
}; // end of UGGridEntity codim = 0

#endif  // #if 0

#endif
