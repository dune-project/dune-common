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
  template<int dim, int dimworld>
  class OneDGridIntersectionIterator :
    public IntersectionIteratorDefault <dim,dimworld,OneDCType,
        OneDGridIntersectionIterator,OneDGridEntity,
        OneDGridElement, OneDGridBoundaryEntity>
  {
    friend class OneDGridEntity<0,dim,dimworld>;
  public:
    //! prefix increment
    OneDGridIntersectionIterator& operator ++();

    //! The default Constructor makes empty Iterator
    OneDGridIntersectionIterator();

    //! The Destructor
    ~OneDGridIntersectionIterator() {};

    //! equality
    bool operator== (const OneDGridIntersectionIterator& i) const;

    //! inequality
    bool operator!= (const OneDGridIntersectionIterator& i) const;

    //! access neighbor, dereferencing
    OneDGridEntity<0,dim,dimworld>& operator*();

    //! access neighbor, arrow
    OneDGridEntity<0,dim,dimworld>* operator->();

    //! return true if intersection is with boundary. \todo connection with
    //! boundary information, processor/outer boundary
    bool boundary ();

    //! return true if across the edge an neighbor on this level exists
    bool neighbor ();

    //! return information about the Boundary
    OneDGridBoundaryEntity<dim,dimworld> & boundaryEntity ();

    //! return unit outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    FieldVector<OneDCType, dimworld>& unit_outer_normal (const FieldVector<OneDCType, dim-1>& local);

    //! return unit outer normal, if you know it is constant use this function instead
    FieldVector<OneDCType, dimworld>& unit_outer_normal ();

    //! intersection of codimension 1 of this neighbor with element where
    //! iteration started.
    //! Here returned element is in LOCAL coordinates of the element
    //! where iteration started.
    OneDGridElement<dim-1,dim>& intersection_self_local ();

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in GLOBAL coordinates of the element where iteration started.
    OneDGridElement<dim-1,dimworld>& intersection_self_global ();

    //! local number of codim 1 entity in self where intersection is contained in
    int number_in_self ();

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in LOCAL coordinates of neighbor
    OneDGridElement<dim-1,dim>& intersection_neighbor_local ();

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in LOCAL coordinates of neighbor
    OneDGridElement<dim-1,dimworld>& intersection_neighbor_global ();

    //! local number of codim 1 entity in neighbor where intersection is contained
    int number_in_neighbor ();

    //! return outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    FieldVector<OneDCType, dimworld>& outer_normal (const FieldVector<OneDCType, dim-1>& local);

    //! return unit outer normal, if you know it is constant use this function instead
    FieldVector<OneDCType, dimworld>& outer_normal ();

  private:
    //**********************************************************
    //  private methods
    //**********************************************************

    OneDGridEntity<0,dim,dimworld> virtualEntity_;

    //! vector storing the outer normal
    FieldVector<OneDCType, dimworld> outerNormal_;

    //! pointer to element holding the self_local and self_global information.
    //! This element is created on demand.
    OneDGridElement<dim-1,dim> fakeNeigh_;

    //! pointer to element holding the neighbor_global and neighbor_local
    //! information. This element is created on demand.
    OneDGridElement<dim-1,dimworld> neighGlob_;

    //! BoundaryEntity
    OneDGridBoundaryEntity<dim,dimworld> boundaryEntity_;

    //! count on which neighbor we are lookin' at
    int neighborCount_;


  };

  //#include "ugintersectionit.cc"

}  // namespace Dune

#endif
