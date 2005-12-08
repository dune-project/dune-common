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
     a neighbor is an entity of codimension 0 which has a common entity of codimension 1
     These neighbors are accessed via a IntersectionIterator. This allows the implement
     non-matching meshes. The number of neigbors may be different from the number
     of an element!
   */
  template<class GridImp>
  class UGGridIntersectionIterator :
    public IntersectionIteratorDefault <GridImp,UGGridIntersectionIterator>
  {

    enum {dim=GridImp::dimension};

    enum {dimworld=GridImp::dimensionworld};

    friend class UGGridEntity<0,dim,GridImp>;

    // The type used to store coordinates
    typedef typename GridImp::ctype UGCtype;

  public:

    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;
    typedef typename GridImp::template Codim<1>::Geometry Geometry;
    typedef typename GridImp::template Codim<1>::LocalGeometry LocalGeometry;
    typedef typename GridImp::template Codim<0>::Entity Entity;

    /** The default Constructor makes empty Iterator
        \todo Should be private
     */
    UGGridIntersectionIterator(typename TargetType<0,GridImp::dimensionworld>::T* center, int nb, int level)
      : center_(center), level_(level), neighborCount_(nb)
    {}

    //! The Destructor
    ~UGGridIntersectionIterator() {};

    int level() const {
      return level_;
    }

    //! equality
    bool equals(const UGGridIntersectionIterator<GridImp>& i) const {
      return center_==i.center_ && neighborCount_ == i.neighborCount_;
    }

    //! prefix increment
    void increment() {
      neighborCount_++;
      if (neighborCount_ >= UG_NS<GridImp::dimensionworld>::Sides_Of_Elem(center_))
        neighborCount_ = -1;
    }

    //! return EntityPointer to the Entity on the inside of this intersection
    //! (that is the Entity where we started this Iterator)
    EntityPointer inside() const {
      UGGridEntityPointer<0,GridImp> center;
      center.setToTarget(center_, this->level());
      return center;
    }

    //! return EntityPointer to the Entity on the outside of this intersection
    //! (that is the neighboring Entity)
    EntityPointer outside() const {
      UGGridEntityPointer<0,GridImp> other;
      typename TargetType<0,GridImp::dimensionworld>::T* otherelem = getNeighbor();
      if (otherelem==0) DUNE_THROW(GridError,"no neighbor found in outside()");
      other.setToTarget(otherelem,UG_NS<GridImp::dimensionworld>::myLevel(otherelem));
      return other;
    }

    //! return true if intersection is with boundary. \todo connection with
    //! boundary information, processor/outer boundary
    bool boundary () const;

    //! return true if across the edge an neighbor on this level exists
    bool neighbor () const;

    //! return information about the Boundary
    int boundaryId () const {
      DUNE_THROW(NotImplemented, "Coarse grid boundary segment id");
      return 0;
    }

    //! intersection of codimension 1 of this neighbor with element where
    //! iteration started.
    //! Here returned element is in LOCAL coordinates of the element
    //! where iteration started.
    const LocalGeometry& intersectionSelfLocal () const;

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in GLOBAL coordinates of the element where iteration started.
    const Geometry& intersectionGlobal () const;

    //! local number of codim 1 entity in self where intersection is contained in
    int numberInSelf () const;

    //! intersection of codimension 1 of this neighbor with element where iteration started.
    //! Here returned element is in LOCAL coordinates of neighbor
    const LocalGeometry& intersectionNeighborLocal () const;

    //! local number of codim 1 entity in neighbor where intersection is contained
    int numberInNeighbor () const;

    //! return outer normal, this should be dependent on local
    //! coordinates for higher order boundary
    const FieldVector<UGCtype, GridImp::dimensionworld>& outerNormal (const FieldVector<UGCtype, GridImp::dimension-1>& local) const;

  private:
    //**********************************************************
    //  private methods
    //**********************************************************

    //! get neighbor on same or lower level or 0
    typename TargetType<0,GridImp::dimensionworld>::T* getNeighbor () const;

    //! renumbering of faces from UG to Dune
    int renumberFaceUGToDune (int nSides, int i) const;

    //! vector storing the outer normal
    mutable FieldVector<UGCtype, dimworld> outerNormal_;

    //! pointer to element holding the self_local and self_global information.
    //! This element is created on demand.
    mutable UGMakeableGeometry<dim-1,dim,GridImp> selfLocal_;
    mutable UGMakeableGeometry<dim-1,dim,GridImp> neighLocal_;

    //! pointer to element holding the neighbor_global and neighbor_local
    //! information. This element is created on demand.
    mutable UGMakeableGeometry<dim-1,dimworld,GridImp> neighGlob_;

    //! This points to the same UG element as center_ does
    typename TargetType<0,GridImp::dimensionworld>::T *center_;

    //! The level we're on
    int level_;

    //! count on which neighbor we are lookin' at. Note that this is interpreted in UG's ordering!
    int neighborCount_;


  };

#include "ugintersectionit.cc"

}  // namespace Dune

#endif
