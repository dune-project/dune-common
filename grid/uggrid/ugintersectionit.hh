// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGINTERSECTIONIT_HH
#define DUNE_UGINTERSECTIONIT_HH

/** \file
 * \brief The UGGridIntersectionIterator class
 */

namespace Dune {

  //**********************************************************************
  //
  // --UGGridIntersectionIterator
  // --IntersectionIterator
  /** \brief Iterator over all element neighbors
   * \ingroup UGGrid
     Mesh entities of codimension 0 ("elements") allow to visit all neighbors, where
     a neighbor is an entity of codimension 0 which has a common entity of codimension
     These neighbors are accessed via a IntersectionIterator. This allows the implement
     non-matching meshes. The number of neigbors may be different from the number
     of an element!
   */
  template<class GridImp>
  class UGGridIntersectionIterator :
    public Dune::UGGridEntityPointer <0,GridImp>,
    public IntersectionIteratorDefault <GridImp,UGGridIntersectionIterator>
  {

    enum {dim=GridImp::dimension};

    enum {dimworld=GridImp::dimensionworld};

    friend class UGGridEntity<0,dim,GridImp>;

  public:

    typedef typename GridImp::template codim<1>::Geometry Geometry;
    typedef typename GridImp::template codim<1>::LocalGeometry LocalGeometry;
    typedef typename GridImp::template codim<0>::Entity Entity;

    //! The default Constructor makes empty Iterator
    UGGridIntersectionIterator();

    //! The Destructor
    ~UGGridIntersectionIterator() {};

    //! prefix increment
    void increment() {
      setToTarget(center_, neighborCount_+1);
    }

    //! equality
    bool equals(const UGGridIntersectionIterator& i) const;

    //! dereferencing
    Entity& dereference() const {
      return virtualEntity_;
    }

    //! return true if intersection is with boundary. \todo connection with
    //! boundary information, processor/outer boundary
    bool boundary () const;

    //! return true if across the edge an neighbor on this level exists
    bool neighbor () const;

    //! return information about the Boundary
    const UGGridBoundaryEntity<GridImp> & boundaryEntity () const;

    //! intersection of codimension 1 of this neighbor with element where
    //! iteration started.
    //! Here returned element is in LOCAL coordinates of the element
    //! where iteration started.
    LocalGeometry& intersectionSelfLocal () const;

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in GLOBAL coordinates of the element where iteration started.
    Geometry& intersectionGlobal () const;

    //! local number of codim 1 entity in self where intersection is contained in
    int numberInSelf () const;

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in LOCAL coordinates of neighbor
    LocalGeometry& intersectionNeighborLocal () const;

    //! local number of codim 1 entity in neighbor where intersection is contained
    int numberInNeighbor () const;

    //! return outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    FieldVector<UGCtype, GridImp::dimensionworld>& outerNormal (const FieldVector<UGCtype, GridImp::dimension-1>& local) const;

    //! return unit outer normal, if you know it is constant use this function instead
    FieldVector<UGCtype, GridImp::dimensionworld>& outerNormal () const;

  private:
    //**********************************************************
    //  private methods
    //**********************************************************

    //! Returns the element whose intersection with center_
    //! is represented by the intersection iterator
    typename TargetType<0,GridImp::dimensionworld>::T* target() const;

    //! Set intersection iterator to nb-th neighbor of element 'center'
    void setToTarget(typename TargetType<0,GridImp::dimensionworld>::T* center, int nb);

    /** \brief Set intersection iterator to nb-th neighbor of element 'center'
     * and set the level as well */
    void setToTarget(typename TargetType<0,GridImp::dimensionworld>::T* center, int nb, int level);

    //! Returns true if the iterator represents an actual intersection
    bool isValid() const;

    mutable UGMakeableEntity<0,dim,GridImp> virtualEntity_;

    //! vector storing the outer normal
    mutable FieldVector<UGCtype, dimworld> outerNormal_;

    //! pointer to element holding the self_local and self_global information.
    //! This element is created on demand.
    UGGridGeometry<dim-1,dim,GridImp> fakeNeigh_;

    //! pointer to element holding the neighbor_global and neighbor_local
    //! information. This element is created on demand.
    mutable UGMakeableGeometry<dim-1,dimworld,GridImp> neighGlob_;

    //! BoundaryEntity
    UGGridBoundaryEntity<GridImp> boundaryEntity_;

    //! The element whose neighbors we are looking at
    typename TargetType<0,dimworld>::T* center_;

    //! count on which neighbor we are lookin' at
    int neighborCount_;


  };

#include "ugintersectionit.cc"

}  // namespace Dune

#endif
