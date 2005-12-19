// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ONE_D_GRID_INTERSECTIONIT_HH
#define DUNE_ONE_D_GRID_INTERSECTIONIT_HH

/** \file
 * \brief The OneDGridIntersectionIterator class
 */

namespace Dune {


  //**********************************************************************
  //
  // --OneDGridIntersectionIterator
  // --IntersectionIterator
  /** \brief Iterator over all element neighbors
   * \ingroup OneDGrid
     Mesh entities of codimension 0 ("elements") allow to visit all neighbors, where
     a neighbor is an entity of codimension 0 which has a common entity of codimension
     These neighbors are accessed via a IntersectionIterator. This allows the implement
     non-matching meshes. The number of neigbors may be different from the number
     of an element!
   */
  template<class GridImp>
  class OneDGridIntersectionIterator :
    public IntersectionIteratorDefault <GridImp, OneDGridIntersectionIterator>
  {
    enum { dim=GridImp::dimension };
    enum { dimworld=GridImp::dimensionworld };

    friend class OneDGridEntity<0,dim,GridImp>;

    //! Constructor for a given grid entity
    OneDGridIntersectionIterator(OneDEntityImp<1>* center, int nb) : center_(center), neighbor_(nb)
    {}

  public:

    typedef typename GridImp::template Codim<1>::Geometry Geometry;
    typedef typename GridImp::template Codim<1>::LocalGeometry LocalGeometry;
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;
    typedef typename GridImp::template Codim<0>::Entity Entity;

    //! The Destructor
    ~OneDGridIntersectionIterator() {};

    //! equality
    bool equals(const OneDGridIntersectionIterator<GridImp>& other) const {
      return (center_ == other.center_) && (neighbor_ == other.neighbor_);
    }

    //! prefix increment
    void increment() {
      neighbor_++;
    }

    OneDEntityImp<1>* target() const {
      const bool isValid = center_ && (neighbor_==0 || neighbor_==1);

      if (!isValid)
        return center_;
      else if (!neighbor())
        return NULL;
      else
        return (neighbor_==0) ? center_->pred_ : center_->succ_;
    }

    //! return true if intersection is with boundary.
    bool boundary () const {
      assert(neighbor_==0 || neighbor_==1);

      // Check whether we're on the left boundary
      if (neighbor_==0) {

        // If there's an element to the left we can't be on the boundary
        if (center_->pred_)
          return false;

        const OneDEntityImp<1>* ancestor = center_;

        while (ancestor->level_!=0) {

          // Check if we're the left son of our father
          if (ancestor != ancestor->father_->sons_[0])
            return false;

          ancestor = ancestor->father_;
        }

        // We have reached level 0.  If there is no element of the left
        // we're truly on the boundary
        return !ancestor->pred_;
      }

      // ////////////////////////////////
      //   Same for the right boundary
      // ////////////////////////////////
      // If there's an element to the right we can't be on the boundary
      if (center_->succ_)
        return false;

      const OneDEntityImp<1>* ancestor = center_;

      while (ancestor->level_!=0) {

        // Check if we're the left son of our father
        if (ancestor != ancestor->father_->sons_[1])
          return false;

        ancestor = ancestor->father_;
      }

      // We have reached level 0.  If there is no element of the left
      // we're truly on the boundary
      return !ancestor->succ_;

    }

    //! return true if across the edge an neighbor on this level exists
    bool neighbor () const {
      if (neighbor_==0)
        return center_->pred_ && center_->pred_->vertex_[1] == center_->vertex_[0];
      else
        return center_->succ_ && center_->succ_->vertex_[0] == center_->vertex_[1];
    }

    //! return EntityPointer to the Entity on the inside of this intersection
    //! (that is the Entity where we started this Iterator)
    EntityPointer inside() const
    {
      return OneDGridEntityPointer<0,GridImp>(center_);
    }

    //! return EntityPointer to the Entity on the outside of this intersection
    //! (that is the neighboring Entity)
    EntityPointer outside() const
    {
      return OneDGridEntityPointer<0,GridImp>(target());
    }

    //! ask for level of entity
    int level () const {return center_->level_;}

    //! return information about the Boundary
    int boundaryId () const {
      return 1;
    }

    //! intersection of codimension 1 of this neighbor with element where
    //! iteration started.
    //! Here returned element is in LOCAL coordinates of the element
    //! where iteration started.
    LocalGeometry& intersectionSelfLocal () const {
      DUNE_THROW(NotImplemented, "Will be implemented on demand!");
    }

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in LOCAL coordinates of neighbor
    LocalGeometry& intersectionNeighborLocal () const {
      DUNE_THROW(NotImplemented, "Will be implemented on demand!");
    }

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in GLOBAL coordinates of the element where iteration started.
    Geometry& intersectionGlobal () const {
      DUNE_THROW(NotImplemented, "Will be implemented on demand!");
    }

    //! local number of codim 1 entity in self where intersection is contained in
    int numberInSelf () const {return neighbor_;}

    //! local number of codim 1 entity in neighbor where intersection is contained
    int numberInNeighbor () const {
      DUNE_THROW(NotImplemented, "number_in_neighbor");
    }

    //! return outer normal
    const FieldVector<OneDCType, dimworld>& outerNormal (const FieldVector<OneDCType, dim-1>& local) const {
      outerNormal_[0] = (neighbor_==0) ? -1 : 1;
      return outerNormal_;
    }

    //! return unit outer normal, this should be dependent on
    //! local coordinates for higher order boundary
    //! the normal is scaled with the integration element
    const FieldVector<OneDCType, dimworld>& integrationOuterNormal (const FieldVector<OneDCType, dim-1>& local) const
    {
      outerNormal_[0] = (neighbor_==0) ? -1 : 1;
      return outerNormal_;
    }

    //! return unit outer normal
    const FieldVector<OneDCType, dimworld>& unitOuterNormal (const FieldVector<OneDCType, dim-1>& local) const {
      outerNormal_[0] = (neighbor_==0) ? -1 : 1;
      return outerNormal_;
    }

  private:
    //**********************************************************
    //  private methods
    //**********************************************************

    OneDEntityImp<1>* center_;

    //! vector storing the outer normal
    mutable FieldVector<OneDCType, dimworld> outerNormal_;

    //! count on which neighbor we are lookin' at
    int neighbor_;

  };

}  // namespace Dune

#endif
