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
    OneDGridIntersectionIterator(OneDGridEntity<0,1,GridImp>* center) : center_(center), neighbor_(0)
    {}

    //! The default Constructor makes empty Iterator
    OneDGridIntersectionIterator();

  public:

    //! The Destructor
    ~OneDGridIntersectionIterator() {};

    //! prefix increment
    OneDGridIntersectionIterator& operator ++() {
      if (center_ && neighbor_==0) {
        neighbor_ = 1;
      } else if (center_ && neighbor_==1) {
        center_ = NULL;
        neighbor_ = -1;
      }
      return *this;
    }

    //! equality
    bool operator== (const OneDGridIntersectionIterator& i) const {
      bool isValid = center_ && (neighbor_==0 || neighbor_==1);
      bool iisValid = i.center_ && (i.neighbor_==0 || i.neighbor_==1);

      // Two intersection iterators are equal if they have the same
      // validity.  Furthermore, if they are both valid, they have
      // to have the same center and neighborCount_
      return (!isValid && !iisValid)
             || (isValid && iisValid &&
                 (center_ == i.center_ && neighbor_ == i.neighbor_));

    }

    //! inequality
    bool operator!= (const OneDGridIntersectionIterator& i) const {
      return !((*this)==i);
    }

    //! access neighbor, dereferencing
    OneDGridEntity<0,dim,GridImp>& operator*(){
      if (neighbor_==0)
        if (center_->pred_ && center_->pred_->geo_.vertex[1] == center_->geo_.vertex[0])
          return *center_->pred_;
        else
        if (center_->succ_ && center_->succ_->geo_.vertex[0] == center_->geo_.vertex[1])
          return *center_->succ_;

      DUNE_THROW(GridError, "Trying to dereferentiate a NULL-pointer!");
    }

    //! access neighbor, arrow
    OneDGridEntity<0,dim,GridImp>* operator->() {
      if (neighbor_==0)
        if (center_->pred_ && center_->pred_->geo_.vertex[1] == center_->geo_.vertex[0])
          return center_->pred_;
        else
        if (center_->succ_ && center_->succ_->geo_.vertex[0] == center_->geo_.vertex[1])
          return center_->succ_;

      return 0;
    }

    //! return true if intersection is with boundary.
    bool boundary () {
      assert(neighbor_==0 || neighbor_==1);

      // Check whether we're on the left boundary
      if (neighbor_==0) {

        // If there's an element to the left we can't be on the boundary
        if (center_->pred_)
          return false;

        OneDGridEntity<0,dim,GridImp>* ancestor = center_;

        while (ancestor->level()!=0) {

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

      OneDGridEntity<0,dim,GridImp>* ancestor = center_;

      while (ancestor->level()!=0) {

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
        return center_->pred_ && center_->pred_->geo_.vertex[1] == center_->geo_.vertex[0];
      else
        return center_->succ_ && center_->succ_->geo_.vertex[0] == center_->geo_.vertex[1];
    }

    //! return information about the Boundary
    OneDGridBoundaryEntity<GridImp> & boundaryEntity () {
      return boundaryEntity_;
    }

    //! return unit outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    FieldVector<OneDCType, dimworld>& unit_outer_normal (const FieldVector<OneDCType, dim-1>& local);

    //! return unit outer normal, if you know it is constant use this function instead
    FieldVector<OneDCType, dimworld>& unit_outer_normal ();

    //! intersection of codimension 1 of this neighbor with element where
    //! iteration started.
    //! Here returned element is in LOCAL coordinates of the element
    //! where iteration started.
    OneDGridGeometry<dim-1,dim,GridImp>& intersectionSelfLocal ();

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in GLOBAL coordinates of the element where iteration started.
    OneDGridGeometry<dim-1,dimworld, GridImp>& intersectionGlobal ();

    //! local number of codim 1 entity in self where intersection is contained in
    int number_in_self () const {return neighbor_;}

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in LOCAL coordinates of neighbor
    OneDGridGeometry<dim-1,dim, GridImp>& intersectionNeighborLocal ();

    //! local number of codim 1 entity in neighbor where intersection is contained
    int number_in_neighbor () const {
      DUNE_THROW(NotImplemented, "number_in_neighbor");
    }

    //! return outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    const FieldVector<OneDCType, dimworld>& outer_normal (const FieldVector<OneDCType, dim-1>& local) const {
      return outer_normal();
    }

    //! return unit outer normal, if you know it is constant use this function instead
    const FieldVector<OneDCType, dimworld>& outer_normal () const {
      outerNormal_[0] = (neighbor_==0) ? -1 : 1;
      return outerNormal_;
    }

  private:
    //**********************************************************
    //  private methods
    //**********************************************************

    OneDGridEntity<0,dim,GridImp>* center_;

    //! vector storing the outer normal
    FieldVector<OneDCType, dimworld> outerNormal_;

#if 0
    //! pointer to element holding the self_local and self_global information.
    //! This element is created on demand.
    OneDGridGeometry<dim-1,dim> fakeNeigh_;

    //! pointer to element holding the neighbor_global and neighbor_local
    //! information. This element is created on demand.
    OneDGridGeometry<dim-1,dimworld> neighGlob_;
#endif

    //! BoundaryEntity
    OneDGridBoundaryEntity<GridImp> boundaryEntity_;

    //! count on which neighbor we are lookin' at
    int neighbor_;


  };

  //#include "ugintersectionit.cc"

}  // namespace Dune

#endif
