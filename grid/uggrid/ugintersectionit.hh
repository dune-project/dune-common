// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGINTERSECTIONIT_HH
#define DUNE_UGINTERSECTIONIT_HH

namespace Dune {

  //**********************************************************************
  //
  // --UGGridIntersectionIterator
  // --IntersectionIterator
  /*!
     Mesh entities of codimension 0 ("elements") allow to visit all neighbors, where
     a neighbor is an entity of codimension 0 which has a common entity of codimension
     These neighbors are accessed via a IntersectionIterator. This allows the implement
     non-matching meshes. The number of neigbors may be different from the number
     of an element!
   */
  template<int dim, int dimworld>
  class UGGridIntersectionIterator :
    public IntersectionIteratorDefault <dim,dimworld,UGCtype,
        UGGridIntersectionIterator,UGGridEntity,
        UGGridElement, UGGridBoundaryEntity>
  {
    friend class UGGridEntity<0,dim,dimworld>;
  public:
    //! prefix increment
    UGGridIntersectionIterator& operator ++();

    //! The default Constructor makes empty Iterator
    UGGridIntersectionIterator();

    //! The Destructor
    ~UGGridIntersectionIterator() {};

    //! equality
    bool operator== (const UGGridIntersectionIterator& i) const;

    //! inequality
    bool operator!= (const UGGridIntersectionIterator& i) const;

    //! access neighbor, dereferencing
    UGGridEntity<0,dim,dimworld>& operator*();

    //! access neighbor, arrow
    UGGridEntity<0,dim,dimworld>* operator->();

    //! return true if intersection is with boundary. \todo connection with
    //! boundary information, processor/outer boundary
    bool boundary ();

    //! return true if across the edge an neighbor on this level exists
    bool neighbor ();

    //! return information about the Boundary
    UGGridBoundaryEntity<dim,dimworld> & boundaryEntity ();

    //! return unit outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    FieldVector<UGCtype, dimworld>& unit_outer_normal (const FieldVector<UGCtype, dim-1>& local);

    //! return unit outer normal, if you know it is constant use this function instead
    FieldVector<UGCtype, dimworld>& unit_outer_normal ();

    //! intersection of codimension 1 of this neighbor with element where
    //! iteration started.
    //! Here returned element is in LOCAL coordinates of the element
    //! where iteration started.
    UGGridElement<dim-1,dim>& intersection_self_local ();

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in GLOBAL coordinates of the element where iteration started.
    UGGridElement<dim-1,dimworld>& intersection_self_global ();

    //! local number of codim 1 entity in self where intersection is contained in
    int number_in_self ();

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in LOCAL coordinates of neighbor
    UGGridElement<dim-1,dim>& intersection_neighbor_local ();

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in LOCAL coordinates of neighbor
    UGGridElement<dim-1,dimworld>& intersection_neighbor_global ();

    //! local number of codim 1 entity in neighbor where intersection is contained
    int number_in_neighbor ();

    //! return outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    FieldVector<UGCtype, dimworld>& outer_normal (const FieldVector<UGCtype, dim-1>& local);

    //! return unit outer normal, if you know it is constant use this function instead
    FieldVector<UGCtype, dimworld>& outer_normal ();

  private:
    //**********************************************************
    //  private methods
    //**********************************************************

    //! Returns the element whose intersection with center_
    //! is represented by the intersection iterator
    typename TargetType<0,dimworld>::T* target() const;

    //! Set intersection iterator to nb-th neighbor of element 'center'
    void setToTarget(typename TargetType<0,dimworld>::T* center, int nb);

    //! Returns true if the iterator represents an actual intersection
    bool isValid() const;

    //! the actual level
    //int level_;

    UGGridEntity<0,dim,dimworld> virtualEntity_;

    //! vector storing the outer normal
    FieldVector<UGCtype, dimworld> outerNormal_;

    //! pointer to element holding the self_local and self_global information.
    //! This element is created on demand.
    UGGridElement<dim-1,dim> *fakeNeigh_;

    //! pointer to element holding the neighbor_global and neighbor_local
    //! information. This element is created on demand.
    UGGridElement<dim-1,dimworld> *neighGlob_;

    //! BoundaryEntity
    UGGridBoundaryEntity<dim,dimworld> boundaryEntity_;

    //! The element whose neighbors we are looking at
    typename TargetType<0,dimworld>::T* center_;

    //! count on which neighbor we are lookin' at
    int neighborCount_;


  };

};  // namespace Dune


#include "ugintersectionit.cc"



#endif
