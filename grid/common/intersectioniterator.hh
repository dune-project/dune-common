// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_INTERSECTIONITERATOR_HH
#define DUNE_GRID_INTERSECTIONITERATOR_HH

#include <dune/common/iteratorfacades.hh>

namespace Dune
{

  //************************************************************************
  // IntersectionIterator
  //************************************************************************

  template<class GridImp, template<class> class IntersectionIteratorImp>
  class IntersectionIterator :
    public Dune::ForwardIteratorFacade<IntersectionIterator<GridImp,IntersectionIteratorImp>,
        typename GridImp::template codim<0>::Entity>
  {
    enum { dim=GridImp::dimension };
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  protected:
    IntersectionIteratorImp<GridImp> realIterator;
  public:
    typedef typename GridImp::template codim<0>::Entity Entity;
    typedef typename GridImp::template codim<0>::BoundaryEntity BoundaryEntity;
    typedef typename GridImp::template codim<1>::Geometry Geometry;
    typedef typename GridImp::template codim<1>::LocalGeometry LocalGeometry;
    //! know your own dimension
    enum { dimension=dim };
    //! know your own dimension of world
    enum { dimensionworld=dimworld };
    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! increment
    void increment()
    {
      realIterator.increment();
    }
    //! equality
    bool equals (const IntersectionIterator<GridImp,IntersectionIteratorImp>& i) const
    {
      return realIterator.equals(i.realIterator);
    }
    //! access neighbor, dereferencing
    Entity& dereference() const
    {
      return realIterator.dereference();
    }
    //! return true if intersection is with boundary. \todo connection with boundary information, processor/outer boundary
    bool boundary () const
    {
      return realIterator.boundary();
    }
    const BoundaryEntity & boundaryEntity () const
    {
      return realIterator.boundaryEntity();
    }
    //! return true if intersection is with neighbor on this level.
    bool neighbor () const
    {
      return realIterator.neighbor();
    }
    /*! return an outer normal (length not necessarily 1)

       The returned Vector is copied to take advantage from the return type
       optimization.  Usually you will use the normal in several
       calculations, so that you store it before you use it.
     */
    FieldVector<ct, dimworld> outerNormal (FieldVector<ct, dim-1>& local) const
    {
      realIterator.fill_outer_normal(local);
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of the element where iteration started.
     */
    const LocalGeometry& intersectionSelfLocal () const
    {
      return realIterator.intersectionSelfLocal();
    }
    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of neighbor
     */
    const LocalGeometry& intersectionNeighborLocal () const
    {
      return realIterator.intersectionNeighborLocal();
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in GLOBAL coordinates of the element where iteration started.
     */
    const Geometry& intersectionGlobal () const
    {
      return realIterator.intersectionGlobal();
    }

    //! local number of codim 1 entity in self where intersection is contained in
    int numberInSelf () const
    {
      return realIterator.numberInSelf ();
    }

    //! local number of codim 1 entity in neighbor where intersection is contained in
    int numberInNeighbor () const
    {
      return realIterator.numberInNeighbor ();
    }

    //! return unit outer normal, this should be dependent on
    //! local coordinates for higher order boundary
    //! the normal is scaled with the integration element
    FieldVector<ct, dimworld> integrationOuterNormal (FieldVector<ct, dim-1>& local) const
    {
      return realIterator.integrationOuterNormal(local);
    }
    //! return unit outer normal
    FieldVector<ct, dimworld> unitOuterNormal (FieldVector<ct, dim-1>& local) const
    {
      return realIterator.unitOuterNormal(local);
    }

    // copy constructor from IntersectionIteratorImp
    IntersectionIterator (const IntersectionIteratorImp<const GridImp> & i) :
      realIterator(i) {}
  };

  //************************************************************************
  // IntersectionIteratorInterface
  //************************************************************************

  /*! Mesh entities of codimension 0 ("elements") allow to visit all neighbors, where
     a neighbor is an entity of codimension 0 which has a common entity of codimension 1 with the entity.
     These neighbors are accessed via a IntersectionIterator. This allows the implementation of
     non-matching meshes. The number of neigbors may be different from the number of faces/edges
     of an element!
   */
  template<class GridImp, template<class> class IntersectionIteratorImp>
  class IntersectionIteratorInterface
  {
    enum { dim=GridImp::dimension };
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  public:
    typedef typename GridImp::template codim<0>::Entity Entity;
    typedef typename GridImp::template codim<0>::BoundaryEntity BoundaryEntity;
    typedef typename GridImp::template codim<1>::Geometry Geometry;
    typedef typename GridImp::template codim<1>::LocalGeometry LocalGeometry;
    //! know your own dimension
    enum { dimension=dim };
    //! know your own dimension of world
    enum { dimensionworld=dimworld };
    //! define type used for coordinates in grid module
    typedef ct ctype;

    //! increment
    void increment()
    {
      asImp().increment();
    }
    //! equality
    bool equals(const IntersectionIteratorImp<GridImp>& i) const
    {
      return asImp().equals(i);
    }
    //! access neighbor, dereferencing
    Entity& dereference() const
    {
      return asImp().dereference();
    }

    //! return true if intersection is with boundary. \todo connection with boundary information, processor/outer boundary
    bool boundary () const
    {
      return asImp().boundary();
    }
    const BoundaryEntity & boundaryEntity () const
    {
      return asImp().boundaryEntity();
    }

    //! return true if intersection is with neighbor on this level.
    bool neighbor () const
    {
      return asImp().neighbor();
    }

    /*! return an outer normal (length not necessarily 1)

       The returned Vector is copied to take advantage from the return type
       optimization.  Usually you will use the normal in several
       calculations, so that you store it before you use it.
     */
    FieldVector<ct, dimworld> outerNormal (FieldVector<ct, dim-1>& local) const
    {
      asImp().fill_outer_normal(local);
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of the element where iteration started.
     */
    const LocalGeometry& intersectionSelfLocal () const
    {
      return asImp().intersectionSelfLocal();
    }
    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in LOCAL coordinates of neighbor
     */
    const LocalGeometry& intersectionNeighborLocal () const
    {
      return asImp().intersectionNeighborLocal();
    }

    /*! intersection of codimension 1 of this neighbor with element where iteration started.
       Here returned element is in GLOBAL coordinates of the element where iteration started.
     */
    const Geometry& intersectionGlobal () const
    {
      return asImp().intersectionGlobal();
    }

    //! local number of codim 1 entity in self where intersection is contained in
    int numberInSelf () const
    {
      return asImp().numberInSelf ();
    }

    //! local number of codim 1 entity in neighbor where intersection is contained in
    int numberInNeighbor () const
    {
      return asImp().numberInNeighbor ();
    }

  private:
    //!  Barton-Nackman trick
    IntersectionIteratorImp<GridImp>& asImp ()
    {return static_cast<IntersectionIteratorImp<GridImp>&>(*this);}
    const IntersectionIteratorImp<GridImp>& asImp () const
    {return static_cast<const IntersectionIteratorImp<GridImp>&>(*this);}
  };

  //**************************************************************************
  //
  // --IntersectionIteratorDefault
  //
  //! Default implementation for IntersectionIterator.
  //
  //**************************************************************************
  template<class GridImp, template<class> class IntersectionIteratorImp>
  class IntersectionIteratorDefault
    : public IntersectionIteratorInterface <GridImp,IntersectionIteratorImp>
  {
    enum { dim=GridImp::dimension };
    enum { dimworld=GridImp::dimensionworld };
    typedef typename GridImp::ctype ct;
  public:
    //! return unit outer normal, this should be dependent on
    //! local coordinates for higher order boundary
    //! the normal is scaled with the integration element
    FieldVector<ct, dimworld> integrationOuterNormal (FieldVector<ct, dim-1>& local) const
    {
      return unitOuterNormal(local) * asImp().intersectionGlobal().integrationElement(local);
    }
    //! return unit outer normal
    FieldVector<ct, dimworld> unitOuterNormal (FieldVector<ct, dim-1>& local) const
    {
      FieldVector<ct, dimworld> n = asImp().outerNormal(local);
      n /= n.two_norm();
      return n;
    }
  private:
    //!  Barton-Nackman trick
    IntersectionIteratorImp<GridImp>& asImp ()
    {return static_cast<IntersectionIteratorImp<GridImp>&>(*this);}
    const IntersectionIteratorImp<GridImp>& asImp () const
    {return static_cast<const IntersectionIteratorImp<GridImp>&>(*this);}
  };

}

#endif // DUNE_GRID_INTERSECTIONITERATOR_HH
