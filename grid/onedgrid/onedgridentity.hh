// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONE_D_GRID_ENTITY_HH
#define DUNE_ONE_D_GRID_ENTITY_HH

#include <dune/common/fixedarray.hh>
#include <dune/common/dlist.hh>

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
  template<int codim, int dim, int dimworld>
  class OneDGridEntity :
    public EntityDefault <codim,dim,dimworld, OneDCType,
        OneDGridEntity,OneDGridElement,OneDGridLevelIterator,
        OneDGridIntersectionIterator,OneDGridHierarchicIterator>
  {

    template <int codim_, int dim_, int dimworld_, PartitionIteratorType PiType_>
    friend class OneDGridLevelIterator;

    friend class OneDGrid<dim,dimworld>;


    //! Constructor with a given grid level
    OneDGridEntity(int level, double coord) : geo_(coord), level_(level), pred_(NULL), succ_(NULL) {}

  public:

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
    template<int cc> OneDGridLevelIterator<cc,dim,dimworld,All_Partition> entity (int i);

    //! geometry of this entity
    const OneDGridElement<dim-codim,dimworld>& geometry () const {return geo_;}

    /** \brief Location of this vertex within a mesh entity of codimension 0 on the coarse grid.
     *
       This can speed up on-the-fly interpolation for linear conforming elements
       Possibly this is sufficient for all applications we want on-the-fly.
     */
    FieldVector<OneDCType, dim>& local ();



  private:

    //! the current geometry
    OneDGridElement<dim-codim,dimworld> geo_;

    FieldVector<OneDCType, dim> localFatherCoords_;

    //! entity number
    int index_;

    //! level
    int level_;

  public:
    //!
    OneDGridEntity<codim,dim,dimworld>* pred_;

    OneDGridEntity<codim,dim,dimworld>* succ_;


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
  template<int dim, int dimworld>
  class OneDGridEntity<0,dim,dimworld> :
    public EntityDefault<0,dim,dimworld, OneDCType,OneDGridEntity,OneDGridElement,
        OneDGridLevelIterator,OneDGridIntersectionIterator,
        OneDGridHierarchicIterator>
  {
    friend class OneDGrid <dim, dimworld>;
    friend class OneDGridIntersectionIterator < dim, dimworld>;
    friend class OneDGridHierarchicIterator < dim, dimworld>;
    friend class OneDGridLevelIterator <0,dim,dimworld,All_Partition>;

    template <int cc_, int dim_, int dimworld_>
    friend class OneDGridSubEntityFactory;

    //typedef typename OneDGrid<dim,dimworld>::ElementContainer::Iterator ElementIterator;
    typedef typename DoubleLinkedList<OneDGridEntity<0,dim,dimworld> >::Iterator ElementIterator;

  public:

    //! The Iterator over neighbors
    typedef OneDGridIntersectionIterator<dim,dimworld> IntersectionIterator;

    //! Iterator over descendants of the entity
    typedef OneDGridHierarchicIterator<dim,dimworld> HierarchicIterator;

    //! Constructo
    OneDGridEntity(int level) : adaptationState(NONE), level_(level), pred_(NULL), succ_(NULL) {
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
    const OneDGridElement<dim,dimworld>& geometry () const {return geo_;}

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
    OneDGridLevelIterator<cc,dim,dimworld, All_Partition> entity (int i) const;

    /*! Intra-level access to neighboring elements. A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor. */
    OneDGridIntersectionIterator<dim,dimworld> ibegin (){
      return OneDGridIntersectionIterator<dim,dimworld>(this);
    }

    //! Reference to one past the last neighbor
    OneDGridIntersectionIterator<dim,dimworld> iend (){
      return OneDGridIntersectionIterator<dim,dimworld>(NULL);
    }

    //! returns true if Entity has children
    bool hasChildren () const {
      return (sons_[0]!=NULL) || (sons_[1]!=NULL);
    }

    //! Inter-level access to father element on coarser grid.
    //! Assumes that meshes are nested.
    OneDGridLevelIterator<0,dim,dimworld,All_Partition> father () {
      return OneDGridLevelIterator<0,dim,dimworld,All_Partition>(father_);
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
    OneDGridElement<dim,dim>& father_relative_local () {
      DUNE_THROW(NotImplemented, "OneDGrid::father_relative_local() not implemented!");
    }

    /*! Inter-level access to son elements on higher levels<=maxlevel.
       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
     */
    OneDGridHierarchicIterator<dim,dimworld> hbegin (int maxlevel) {

      OneDGridHierarchicIterator<dim,dimworld> it(maxlevel);

      if (level()<maxlevel) {

        // Load sons of old target onto the iterator stack
        if (hasChildren()) {
          typename OneDGridHierarchicIterator<dim,dimworld>::StackEntry se0;
          se0.element = sons_[0];
          se0.level   = level() + 1;
          it.elemStack.push(se0);

          typename OneDGridHierarchicIterator<dim,dimworld>::StackEntry se1;
          se1.element = sons_[1];
          se1.level   = level() + 1;
          it.elemStack.push(se1);
        }

      }

      it.target_ = (it.elemStack.empty()) ? NULL : it.elemStack.top().element;

      return it;
    }

    //! Returns iterator to one past the last son
    OneDGridHierarchicIterator<dim,dimworld> hend (int maxlevel) {
      return OneDGridHierarchicIterator<dim,dimworld>(maxlevel);;
    }

    // ***************************************************************
    //  Interface for Adaptation
    // ***************************************************************

    /** \brief Mark entity for refinement
     *
     * \param refCount if >0 mark for refinement, if <0 mark for coarsening
     *
     * \return false, which is not compliant with the official specification!
     */
    bool mark(int refCount) {
      if (refCount < 0)
        adaptationState = COARSEN;
      else if (refCount > 0)
        adaptationState = REFINED;
      else
        adaptationState = NONE;
      return false;
    }

    /** \todo Please doc me! */
    AdaptationState state() const {return adaptationState;}

  private:

    //! the current geometry
    OneDGridElement<dim,dimworld> geo_;

    FixedArray<OneDGridEntity<0,dim,dimworld>*, 2> sons_;

    OneDGridEntity<0,dim,dimworld>* father_;

    //! element number
    int index_;

    //! the level of the entity
    int level_;

    OneDGridElement <dim,dim> fatherReLocal_;

    AdaptationState adaptationState;

  public:

    OneDGridEntity<0,dim,dimworld>* pred_;

    OneDGridEntity<0,dim,dimworld>* succ_;


  }; // end of OneDGridEntity codim = 0

} // namespace Dune

#endif
